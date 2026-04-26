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
				DestroyPool(PoolPair.Value);
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

	FActorPool* Pool = GetOrCreatePool(World, ActorClass);
	if (!Pool)
	{
		return nullptr;
	}

	AActor* Actor = nullptr;

	// 1. 尝试从可用列表中取出一个
	while (!Actor && Pool->AvailableActors.Num() > 0)
	{
		Actor = Pool->AvailableActors.Pop();
		// 如果Actor已被GC，继续取下一个
		if (!IsValid(Actor))
		{
			Actor = nullptr;
		}
	}

	// 2. 没有可用Actor则动态创建
	if (!Actor)
	{
		const bool bCanExpand = (Pool->MaxSize <= 0) || (Pool->InUseActors.Num() < Pool->MaxSize);
		if (bCanExpand)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Owner;
			SpawnParams.Instigator = Instigator;
			Actor = CreatePooledActor(World, ActorClass, SpawnParams);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Pool for %s is full (MaxSize: %d). Consider increasing MaxSize."),
				*ActorClass->GetName(), Pool->MaxSize);
			return nullptr;
		}
	}

	if (!Actor)
	{
		return nullptr;
	}

	// 3. 激活Actor
	ActivatePooledActor(Actor, SpawnTransform, Owner, Instigator);

	// 4. 添加到使用中集合
	Pool->InUseActors.Add(Actor);

	// 5. 网络同步：强制立即更新网络状态
	if (Actor->GetIsReplicated())
	{
		Actor->ForceNetUpdate();
	}

	// 6. 广播委托
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
	FActorPool* Pool = FindPool(World, Actor->GetClass());
	if (Pool && Pool->InUseActors.Contains(Actor))
	{
		// 广播委托
		OnActorReleased.Broadcast(Actor->GetClass(), Actor);

		// 停用Actor
		DeactivatePooledActor(Actor);

		// 从使用集合移除，加入可用列表
		Pool->InUseActors.Remove(Actor);
		Pool->AvailableActors.Add(Actor);
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

	FActorPool* Pool = GetOrCreatePool(World, ActorClass);
	if (!Pool)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < Count; ++i)
	{
		const int32 CurrentTotal = Pool->AvailableActors.Num() + Pool->InUseActors.Num();
		const bool bCanCreate = (Pool->MaxSize <= 0) || (CurrentTotal < Pool->MaxSize);

		if (bCanCreate)
		{
			AActor* NewActor = CreatePooledActor(World, ActorClass, SpawnParams);
			if (NewActor)
			{
				Pool->AvailableActors.Add(NewActor);
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

	TMap<TSubclassOf<AActor>, FActorPool>* Pools = GetPoolsForWorld(World);
	if (!Pools)
	{
		return;
	}

	if (FActorPool* Pool = Pools->Find(ActorClass))
	{
		DestroyPool(*Pool);
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
				DestroyPool(PoolPair.Value);
			}
		}
		WorldPools.Empty();
		return;
	}

	TMap<TSubclassOf<AActor>, FActorPool>* Pools = GetPoolsForWorld(World);
	if (Pools)
	{
		for (auto& PoolPair : *Pools)
		{
			DestroyPool(PoolPair.Value);
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

	const FActorPool* Pool = FindPool(World, ActorClass);
	return Pool ? Pool->AvailableActors.Num() : 0;
}

int32 UPoolSubsystem::GetPoolInUseCount(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass) const
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !ActorClass)
	{
		return 0;
	}

	const FActorPool* Pool = FindPool(World, ActorClass);
	return Pool ? Pool->InUseActors.Num() : 0;
}

int32 UPoolSubsystem::GetPoolTotalCount(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass) const
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !ActorClass)
	{
		return 0;
	}

	const FActorPool* Pool = FindPool(World, ActorClass);
	return Pool ? (Pool->AvailableActors.Num() + Pool->InUseActors.Num()) : 0;
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

	const FActorPool* Pool = FindPool(World, Actor->GetClass());
	if (!Pool)
	{
		return false;
	}

	return Pool->AvailableActors.Contains(Actor) || Pool->InUseActors.Contains(Actor);
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
				DestroyPool(PoolPair.Value);
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

	// 调用接口激活方法
	if (Actor->GetClass()->ImplementsInterface(UPoolableActor::StaticClass()))
	{
		IPoolableActor::Execute_OnActivateFromPool(Actor, SpawnTransform);
	}
}

void UPoolSubsystem::DeactivatePooledActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// 调用接口清理逻辑
	if (Actor->GetClass()->ImplementsInterface(UPoolableActor::StaticClass()))
	{
		IPoolableActor::Execute_OnReturnToPool(Actor);
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

FActorPool* UPoolSubsystem::GetOrCreatePool(UWorld* World, TSubclassOf<AActor> ActorClass)
{
	if (!World || !ActorClass)
	{
		return nullptr;
	}

	TMap<TSubclassOf<AActor>, FActorPool>* Pools = GetPoolsForWorld(World);
	if (!Pools)
	{
		// 为这个World创建新的池映射
		TMap<TSubclassOf<AActor>, FActorPool> NewPools;
		WorldPools.Add(TWeakObjectPtr<UWorld>(World), NewPools);
		Pools = &WorldPools[TWeakObjectPtr<UWorld>(World)];
	}

	if (FActorPool* Existing = Pools->Find(ActorClass))
	{
		return Existing;
	}

	// 创建新池，读取配置
	FActorPool NewPool;
	NewPool.Class = ActorClass;

	const FPoolConfig Config = GetConfigForClass(ActorClass);
	NewPool.InitialSize = Config.InitialSize;
	NewPool.MaxSize = Config.MaxSize;

	// 如果配置中启用了池
	if (Config.bEnablePooling)
	{
		Pools->Add(ActorClass, NewPool);
		return &Pools->FindChecked(ActorClass);
	}

	return nullptr;
}

FActorPool* UPoolSubsystem::FindPool(UWorld* World, TSubclassOf<AActor> ActorClass)
{
	if (!World || !ActorClass)
	{
		return nullptr;
	}

	TMap<TSubclassOf<AActor>, FActorPool>* Pools = GetPoolsForWorld(World);
	if (!Pools)
	{
		return nullptr;
	}

	return Pools->Find(ActorClass);
}

const FActorPool* UPoolSubsystem::FindPool(UWorld* World, TSubclassOf<AActor> ActorClass) const
{
	if (!World || !ActorClass)
	{
		return nullptr;
	}

	const TMap<TSubclassOf<AActor>, FActorPool>* Pools = GetPoolsForWorld(World);
	if (!Pools)
	{
		return nullptr;
	}

	return Pools->Find(ActorClass);
}

void UPoolSubsystem::InternalClearPool(FActorPool& Pool)
{
	// 将使用中Actor全部标记为可用（但不销毁）
	for (AActor* Actor : Pool.InUseActors)
	{
		if (IsValid(Actor))
		{
			DeactivatePooledActor(Actor);
			Pool.AvailableActors.Add(Actor);
		}
	}
	Pool.InUseActors.Empty();
}

void UPoolSubsystem::DestroyPool(FActorPool& Pool)
{
	// 销毁所有可用Actor
	for (AActor* Actor : Pool.AvailableActors)
	{
		if (IsValid(Actor) && !Actor->IsPendingKillPending())
		{
			Actor->Destroy();
		}
	}
	Pool.AvailableActors.Empty();

	// 销毁所有使用中Actor
	for (AActor* Actor : Pool.InUseActors)
	{
		if (IsValid(Actor) && !Actor->IsPendingKillPending())
		{
			Actor->Destroy();
		}
	}
	Pool.InUseActors.Empty();
}

TMap<TSubclassOf<AActor>, FActorPool>* UPoolSubsystem::GetPoolsForWorld(UWorld* World)
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

const TMap<TSubclassOf<AActor>, FActorPool>* UPoolSubsystem::GetPoolsForWorld(UWorld* World) const
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
