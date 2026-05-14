// Fill out your copyright notice in the Description page of Project Settings.

#include "PoolSubsystem.h"

#include "PoolableActor.h"
#include "ObjectPoolConfig.h"
#include "Engine/World.h"

void UPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 绑定世界清理事件，用于在World切换/销毁时释放对应的池
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &UPoolSubsystem::OnWorldCleanup);

	// 自动预热：如果有配置文件，预热所有标记为自动预热的池
	if (CurrentConfig)
	{
		TArray<TSubclassOf<AActor>> AutoPrewarmClasses = CurrentConfig->GetAutoPrewarmClasses();
		if (AutoPrewarmClasses.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("PoolSubsystem: Auto-prewarming %d actor classes..."), AutoPrewarmClasses.Num());

			// 延迟到下一帧执行预热，确保World已完全初始化
			FTimerHandle PrewarmTimerHandle;
			GetGameInstance()->GetTimerManager().SetTimerForNextTick([this, AutoPrewarmClasses]()
			{
				for (TSubclassOf<AActor> ActorClass : AutoPrewarmClasses)
				{
					if (!ActorClass)
					{
						continue;
					}

					FPoolConfig Config = CurrentConfig->GetConfigForClass(ActorClass);
					if (Config.bEnablePooling && Config.bAutoPrewarm && Config.InitialSize > 0)
					{
						// 获取主World（通常是第一个GameWorld）
						UWorld* World = GetGameInstance()->GetWorld();
						if (World && World->GetNetMode() != NM_Client)
						{
							PrewarmPool(World, ActorClass, Config.InitialSize);
							UE_LOG(LogTemp, Log, TEXT("PoolSubsystem: Prewarmed %d instances of %s"),
								Config.InitialSize, *ActorClass->GetName());
						}
					}
				}
			});
		}
	}
}

void UPoolSubsystem::Deinitialize()
{
	// 清理所有World的池
	for (auto& WorldPair : WorldPools)
	{
		if (UWorld* World = WorldPair.Key.Get())
		{
			for (auto& PoolPair : WorldPair.Value)
			{
				if (PoolPair.Value.IsValid())
				{
					DestroyPool(*PoolPair.Value);
				}
			}
		}
	}
	WorldPools.Empty();

	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
	Super::Deinitialize();
}

AActor* UPoolSubsystem::RequestActor(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass,
	const FTransform& SpawnTransform, AActor* Owner, APawn* Instigator)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!ActorClass || !World)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestActor failed: Invalid ActorClass or World"));
		return nullptr;
	}

	// 对象池仅在服务器端管理。客户端调用时返回null（应由服务器生成后复制到客户端）
	if (World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestActor should only be called on server. World: %s"), *World->GetName());
		return nullptr;
	}

	TSharedPtr<FActorPool> Pool = GetOrCreatePool(World, ActorClass);
	if (!Pool.IsValid())
	{
		return nullptr;
	}

	AActor* Actor = nullptr;
	bool bNeedsDynamicCreation = false;

	// 线程安全地从池中取出Actor
	{
		FScopeLock ScopedLock(&Pool->PoolLock);

		// 1. 尝试从可用队列中取出一个
		while (!Actor && !Pool->AvailableActors.IsEmpty())
		{
			TObjectPtr<AActor> PoppedActor;
			if (Pool->AvailableActors.Dequeue(PoppedActor))
			{
				Pool->AvailableCount--; // 更新计数
				// 如果Actor已被GC，继续取下一个
				if (IsValid(PoppedActor))
				{
					Actor = PoppedActor;
				}
			}
		}

		// 2. 检查是否需要动态创建
		if (!Actor)
		{
			const int32 CurrentTotal = Pool->InUseActors.Num();
			const bool bCanExpand = (Pool->MaxSize <= 0) || (CurrentTotal < Pool->MaxSize);
			if (bCanExpand)
			{
				bNeedsDynamicCreation = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Pool for %s is full (MaxSize: %d). Consider increasing MaxSize."),
					*ActorClass->GetName(), Pool->MaxSize);
				return nullptr;
			}
		}
		else
		{
			// 3. 添加到使用中集合
			Pool->InUseActors.Add(Actor);
			Pool->AllPooledActors.Add(Actor);
			Pool->TotalRequests++;
		}
	}

	// 在锁外创建Actor以避免长时间持锁
	if (bNeedsDynamicCreation)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.Instigator = Instigator;
		Actor = CreatePooledActor(World, ActorClass, SpawnParams);

		if (Actor)
		{
			FScopeLock ScopedLock(&Pool->PoolLock);
			Pool->InUseActors.Add(Actor);
			Pool->AllPooledActors.Add(Actor);
			Pool->TotalRequests++;
			Pool->DynamicCreations++;
		}
	}

	if (!Actor)
	{
		return nullptr;
	}

	// 4. 激活Actor（在锁外执行，避免阻塞）
	ActivatePooledActor(Actor, SpawnTransform, Owner, Instigator);

	// 5. 广播委托
	OnActorRequested.Broadcast(ActorClass, Actor);

	return Actor;
}

void UPoolSubsystem::ReleaseActor(UObject* WorldContextObject, AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		// 如果WorldContext无效，尝试从Actor获取World
		World = Actor->GetWorld();
	}

	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReleaseActor failed: Cannot determine World"));
		return;
	}

	// 确保只在服务器端释放
	if (World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReleaseActor should only be called on server."));
		return;
	}

	// 查找所属池
	TSharedPtr<FActorPool> Pool = FindPool(World, Actor->GetClass());
	if (Pool.IsValid())
	{
		bool bWasInUse = false;
		{
			FScopeLock ScopedLock(&Pool->PoolLock);
			bWasInUse = Pool->InUseActors.Contains(Actor);

			if (bWasInUse)
			{
				// 从使用集合移除，加入可用队列
				Pool->InUseActors.Remove(Actor);
				Pool->AvailableActors.Enqueue(Actor);
				Pool->AvailableCount++; // 更新计数
				Pool->TotalReleases++;
			}
		}

		if (bWasInUse)
		{
			// 广播委托（在锁外）
			OnActorReleased.Broadcast(Actor->GetClass(), Actor);

			// 停用Actor（在锁外）
			DeactivatePooledActor(Actor);
		}
		else
		{
			// Actor不在使用中，直接销毁
			UE_LOG(LogTemp, Warning, TEXT("ReleaseActor: Actor %s is not in use, destroying."), *Actor->GetName());
			Actor->Destroy();
		}
	}
	else
	{
		// 如果找不到池，直接销毁
		UE_LOG(LogTemp, Warning, TEXT("ReleaseActor: Actor %s is not managed by pool, destroying."), *Actor->GetName());
		Actor->Destroy();
	}
}

void UPoolSubsystem::PrewarmPool(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, int32 Count)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!ActorClass || !World)
	{
		return;
	}

	if (World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("PrewarmPool should only be called on server."));
		return;
	}

	TSharedPtr<FActorPool> Pool = GetOrCreatePool(World, ActorClass);
	if (!Pool.IsValid())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < Count; ++i)
	{
		int32 CurrentTotal = 0;
		{
			FScopeLock ScopedLock(&Pool->PoolLock);
			CurrentTotal = Pool->AvailableCount + Pool->InUseActors.Num();
		}

		const bool bCanCreate = (Pool->MaxSize <= 0) || (CurrentTotal < Pool->MaxSize);

		if (bCanCreate)
		{
			AActor* NewActor = CreatePooledActor(World, ActorClass, SpawnParams);
			if (NewActor)
			{
				FScopeLock ScopedLock(&Pool->PoolLock);
				Pool->AvailableActors.Enqueue(NewActor);
				Pool->AllPooledActors.Add(NewActor);
				Pool->AvailableCount++; // 更新计数
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("PrewarmPool reached MaxSize for %s"), *ActorClass->GetName());
			break;
		}
	}
}

void UPoolSubsystem::ClearPool(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !ActorClass)
	{
		return;
	}

	TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>>* Pools = GetPoolsForWorld(World);
	if (!Pools)
	{
		return;
	}

	if (TSharedPtr<FActorPool>* PoolPtr = Pools->Find(ActorClass))
	{
		if (PoolPtr->IsValid())
		{
			DestroyPool(**PoolPtr);
		}
		Pools->Remove(ActorClass);

		UE_LOG(LogTemp, Log, TEXT("ClearPool: Destroyed pool for %s in World %s"),
			*ActorClass->GetName(), *World->GetName());
	}
}

void UPoolSubsystem::ClearAllPools(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		// 清理所有World的池
		for (auto& WorldPair : WorldPools)
		{
			for (auto& PoolPair : WorldPair.Value)
			{
				if (PoolPair.Value.IsValid())
				{
					DestroyPool(*PoolPair.Value);
				}
			}
		}
		WorldPools.Empty();
		return;
	}

	TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>>* Pools = GetPoolsForWorld(World);
	if (Pools)
	{
		for (auto& PoolPair : *Pools)
		{
			if (PoolPair.Value.IsValid())
			{
				DestroyPool(*PoolPair.Value);
			}
		}
		Pools->Empty();
	}
}

int32 UPoolSubsystem::GetPoolAvailableCount(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass) const
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !ActorClass)
	{
		return 0;
	}

	TSharedPtr<const FActorPool> Pool = FindPool(World, ActorClass);
	return Pool.IsValid() ? Pool->GetAvailableCount() : 0;
}

int32 UPoolSubsystem::GetPoolInUseCount(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass) const
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !ActorClass)
	{
		return 0;
	}

	TSharedPtr<const FActorPool> Pool = FindPool(World, ActorClass);
	return Pool.IsValid() ? Pool->GetInUseCount() : 0;
}

int32 UPoolSubsystem::GetPoolTotalCount(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass) const
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !ActorClass)
	{
		return 0;
	}

	TSharedPtr<const FActorPool> Pool = FindPool(World, ActorClass);
	return Pool.IsValid() ? Pool->GetTotalCount() : 0;
}

void UPoolSubsystem::GetPoolStats(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass,
	int32& OutTotalRequests, int32& OutTotalReleases, int32& OutDynamicCreations) const
{
	OutTotalRequests = 0;
	OutTotalReleases = 0;
	OutDynamicCreations = 0;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !ActorClass)
	{
		return;
	}

	TSharedPtr<const FActorPool> Pool = FindPool(World, ActorClass);
	if (Pool.IsValid())
	{
		FScopeLock ScopedLock(&Pool->PoolLock);
		OutTotalRequests = Pool->TotalRequests;
		OutTotalReleases = Pool->TotalReleases;
		OutDynamicCreations = Pool->DynamicCreations;
	}
}

void UPoolSubsystem::ReleaseActors(UObject* WorldContextObject, const TArray<AActor*>& Actors)
{
	for (AActor* Actor : Actors)
	{
		ReleaseActor(WorldContextObject, Actor);
	}
}

bool UPoolSubsystem::IsActorPooled(UObject* WorldContextObject, AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		World = Actor->GetWorld();
	}
	if (!World)
	{
		return false;
	}

	TSharedPtr<const FActorPool> Pool = FindPool(World, Actor->GetClass());
	if (!Pool.IsValid())
	{
		return false;
	}

	FScopeLock ScopedLock(&Pool->PoolLock);

	// 使用 AllPooledActors 进行 O(1) 查询
	return Pool->AllPooledActors.Contains(Actor);
}

void UPoolSubsystem::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (!World)
	{
		return;
	}

	// 找到并清理对应World的池
	for (auto It = WorldPools.CreateIterator(); It; ++It)
	{
		if (It->Key.Get() == World)
		{
			for (auto& PoolPair : It->Value)
			{
				if (PoolPair.Value.IsValid())
				{
					DestroyPool(*PoolPair.Value);
				}
			}
			It.RemoveCurrent();
			break;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PoolSubsystem: Cleaned up pools for World %s"), *World->GetName());
}

AActor* UPoolSubsystem::CreatePooledActor(UWorld* World, TSubclassOf<AActor> ActorClass, const FActorSpawnParameters& SpawnParams)
{
	if (!World || !ActorClass)
	{
		return nullptr;
	}

	FActorSpawnParameters Params = SpawnParams;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(ActorClass, FTransform::Identity, Params);
	if (NewActor)
	{
		// 生成时立即设置为池中默认状态（隐藏、禁用碰撞和Tick）
		ResetActorForPool(NewActor);
	}

	return NewActor;
}

void UPoolSubsystem::ActivatePooledActor(AActor* Actor, const FTransform& SpawnTransform, AActor* Owner, APawn* Instigator)
{
	if (!Actor)
	{
		return;
	}

	// 设置变换
	Actor->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::ResetPhysics);

	// 设置所有者和Instigator
	Actor->SetOwner(Owner);
	if (Instigator)
	{
		Actor->SetInstigator(Instigator);
	}

	// 激活Actor（显示、启用碰撞、Tick等）
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	// 调用接口激活方法（更安全的检查方式）
	if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(Actor))
	{
		PoolableActor->Execute_OnActivateFromPool(Actor, SpawnTransform);
	}
}

void UPoolSubsystem::DeactivatePooledActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// 调用接口清理逻辑（更安全的检查方式）
	if (IPoolableActor* PoolableActor = Cast<IPoolableActor>(Actor))
	{
		PoolableActor->Execute_OnReturnToPool(Actor);
	}

	// 重置Actor状态回池
	ResetActorForPool(Actor);
}

void UPoolSubsystem::ResetActorForPool(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// FIX: Detach from parent before resetting, so re-attachment works correctly
	Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 隐藏Actor并禁用碰撞和Tick
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);

	// 清除所有者和Instigator，防止残留引用
	Actor->SetOwner(nullptr);
	Actor->SetInstigator(nullptr);

	// 将Actor移动到远离视线的位置，避免在隐藏前渲染一帧
	Actor->SetActorLocation(FVector(0.0f, 0.0f, -100000.0f));

	// 如果有根组件，重置物理状态
	if (USceneComponent* RootComp = Actor->GetRootComponent())
	{
		if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(RootComp))
		{
			PrimitiveComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
			PrimitiveComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		}
		RootComp->SetRelativeScale3D(FVector::OneVector);
	}
}

TSharedPtr<FActorPool> UPoolSubsystem::GetOrCreatePool(UWorld* World, TSubclassOf<AActor> ActorClass)
{
	if (!World || !ActorClass)
	{
		return nullptr;
	}

	TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>>* Pools = GetPoolsForWorld(World);
	if (!Pools)
	{
		// 为这个World创建新的池映射
		TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>> NewPools;
		WorldPools.Add(TWeakObjectPtr<UWorld>(World), NewPools);
		Pools = &WorldPools[TWeakObjectPtr<UWorld>(World)];
	}

	if (TSharedPtr<FActorPool>* Existing = Pools->Find(ActorClass))
	{
		return *Existing;
	}

	// 创建新池，读取配置
	TSharedPtr<FActorPool> NewPool = MakeShared<FActorPool>();
	NewPool->Class = ActorClass;

	const FPoolConfig Config = GetConfigForClass(ActorClass);
	NewPool->InitialSize = Config.InitialSize;
	NewPool->MaxSize = Config.MaxSize;

	// 如果配置中启用了池
	if (Config.bEnablePooling)
	{
		Pools->Add(ActorClass, NewPool);
		return NewPool;
	}

	return nullptr;
}

TSharedPtr<FActorPool> UPoolSubsystem::FindPool(UWorld* World, TSubclassOf<AActor> ActorClass)
{
	if (!World || !ActorClass)
	{
		return nullptr;
	}

	TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>>* Pools = GetPoolsForWorld(World);
	if (!Pools)
	{
		return nullptr;
	}

	TSharedPtr<FActorPool>* Found = Pools->Find(ActorClass);
	return Found ? *Found : nullptr;
}

TSharedPtr<const FActorPool> UPoolSubsystem::FindPool(UWorld* World, TSubclassOf<AActor> ActorClass) const
{
	if (!World || !ActorClass)
	{
		return nullptr;
	}

	const TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>>* Pools = GetPoolsForWorld(World);
	if (!Pools)
	{
		return nullptr;
	}

	const TSharedPtr<FActorPool>* Found = Pools->Find(ActorClass);
	return Found ? *Found : nullptr;
}

void UPoolSubsystem::InternalClearPool(FActorPool& Pool)
{
	FScopeLock ScopedLock(&Pool.PoolLock);

	// 将使用中Actor全部标记为可用（但不销毁）
	for (AActor* Actor : Pool.InUseActors)
	{
		if (IsValid(Actor))
		{
			DeactivatePooledActor(Actor);
			Pool.AvailableActors.Enqueue(Actor);
			Pool.AvailableCount++; // 更新计数
		}
	}
	Pool.InUseActors.Empty();
	// AllPooledActors 保持不变，因为 Actor 仍在池中
}

void UPoolSubsystem::DestroyPool(FActorPool& Pool)
{
	FScopeLock ScopedLock(&Pool.PoolLock);

	// 销毁所有可用Actor
	TObjectPtr<AActor> Actor;
	while (Pool.AvailableActors.Dequeue(Actor))
	{
		if (IsValid(Actor))
		{
			Pool.AllPooledActors.Remove(Actor);
			Actor->Destroy();
		}
	}
	Pool.AvailableCount = 0; // 重置计数

	// 销毁所有使用中Actor
	for (AActor* InUseActor : Pool.InUseActors)
	{
		if (IsValid(InUseActor))
		{
			Pool.AllPooledActors.Remove(InUseActor);
			InUseActor->Destroy();
		}
	}
	Pool.InUseActors.Empty();
	Pool.AllPooledActors.Empty();
}

TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>>* UPoolSubsystem::GetPoolsForWorld(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	TWeakObjectPtr<UWorld> WorldPtr(World);
	if (auto* Found = WorldPools.Find(WorldPtr))
	{
		return Found;
	}
	return nullptr;
}

const TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>>* UPoolSubsystem::GetPoolsForWorld(UWorld* World) const
{
	if (!World)
	{
		return nullptr;
	}

	TWeakObjectPtr<UWorld> WorldPtr(World);
	if (const auto* Found = WorldPools.Find(WorldPtr))
	{
		return Found;
	}
	return nullptr;
}

FPoolConfig UPoolSubsystem::GetConfigForClass(TSubclassOf<AActor> ActorClass) const
{
	if (CurrentConfig)
	{
		return CurrentConfig->GetConfigForClass(ActorClass);
	}
	return FPoolConfig();
}
