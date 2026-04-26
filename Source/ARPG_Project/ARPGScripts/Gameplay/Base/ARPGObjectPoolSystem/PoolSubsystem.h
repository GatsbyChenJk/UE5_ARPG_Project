// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolConfig.h"
#include "Engine/EngineTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PoolSubsystem.generated.h"

class UObjectPoolConfig;
class IPoolableActor;
class APawn;

/**
 * 单个Actor池的数据结构
 */
USTRUCT()
struct FActorPool
{
	GENERATED_BODY()

	/** 当前可用的Actor列表 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> AvailableActors;

	/** 当前正在使用的Actor集合 */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> InUseActors;

	/** 初始池大小 */
	int32 InitialSize = 0;

	/** 最大池容量（0表示无限制） */
	int32 MaxSize = 100;

	/** 对应Actor类 */
	TSubclassOf<AActor> Class;

	FActorPool() : InitialSize(0), MaxSize(100), Class(nullptr) {}
};

/** Actor从池中取出时委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoolActorRequested, TSubclassOf<AActor>, ActorClass, AActor*, SpawnedActor);

/** Actor返回池中时委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoolActorReleased, TSubclassOf<AActor>, ActorClass, AActor*, ReleasedActor);

/**
 * 对象池子系统 - 管理游戏中可复用Actor的池化
 * 继承自UGameInstanceSubsystem，支持跨关卡生命周期管理
 * 网络同步设计：仅在服务器端管理池，客户端通过Actor复制接收
 */
UCLASS()
class ARPG_PROJECT_API UPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// === 池操作接口 ===

	/**
	 * 从池中请求一个Actor。如果没有可用Actor且未达到上限，会动态创建新的。
	 * 仅在服务器端执行实际生成逻辑，客户端调用会自动转发到服务器。
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	AActor* RequestActor(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass,
		const FTransform& SpawnTransform, AActor* Owner = nullptr, APawn* Instigator = nullptr);

	/**
	 * 将Actor释放回池中。如果Actor不属于任何池，则直接销毁。
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	void ReleaseActor(UObject* WorldContextObject, AActor* Actor);

	/**
	 * 预热池，预先创建指定数量的Actor。
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	void PrewarmPool(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, int32 Count);

	/**
	 * 清空指定类型的Actor池，销毁所有管理的Actor。
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	void ClearPool(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass);

	/**
	 * 清空指定World中的所有池。
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	void ClearAllPools(UObject* WorldContextObject);

	// === 查询接口 ===

	/** 获取池中可用Actor数量 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	int32 GetPoolAvailableCount(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass) const;

	/** 获取池中正在使用的Actor数量 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	int32 GetPoolInUseCount(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass) const;

	/** 获取池总大小（可用 + 使用中） */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	int32 GetPoolTotalCount(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass) const;

	/** 检查Actor是否由池管理 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool", meta = (WorldContext = "WorldContextObject"))
	bool IsActorPooled(UObject* WorldContextObject, AActor* Actor) const;

	// === 配置接口 ===

	/** 设置池配置文件 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void SetPoolConfig(UObjectPoolConfig* InConfig) { CurrentConfig = InConfig; }

	/** 获取当前池配置 */
	UFUNCTION(BlueprintPure, Category = "ObjectPool")
	UObjectPoolConfig* GetPoolConfig() const { return CurrentConfig; }

	// === 委托 ===

	/** Actor从池中被请求时触发 */
	UPROPERTY(BlueprintAssignable, Category = "ObjectPool")
	FOnPoolActorRequested OnActorRequested;

	/** Actor返回池中时触发 */
	UPROPERTY(BlueprintAssignable, Category = "ObjectPool")
	FOnPoolActorReleased OnActorReleased;

protected:
	/** 当前池配置 */
	UPROPERTY(Transient)
	TObjectPtr<UObjectPoolConfig> CurrentConfig;

	/** 世界切换时清理旧世界的池 */
	UFUNCTION()
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

private:
	/** 按World存储的池映射。使用TWeakObjectPtr避免阻止World GC */
	TMap<TWeakObjectPtr<UWorld>, TMap<TSubclassOf<AActor>, FActorPool>> WorldPools;

	AActor* CreatePooledActor(UWorld* World, TSubclassOf<AActor> ActorClass, const FActorSpawnParameters& SpawnParams);
	void ActivatePooledActor(AActor* Actor, const FTransform& SpawnTransform, AActor* Owner, APawn* Instigator);
	void DeactivatePooledActor(AActor* Actor);
	void ResetActorForPool(AActor* Actor);

	FActorPool* GetOrCreatePool(UWorld* World, TSubclassOf<AActor> ActorClass);
	FActorPool* FindPool(UWorld* World, TSubclassOf<AActor> ActorClass);
	const FActorPool* FindPool(UWorld* World, TSubclassOf<AActor> ActorClass) const;

	void InternalClearPool(FActorPool& Pool);
	void DestroyPool(FActorPool& Pool);

	TMap<TSubclassOf<AActor>, FActorPool>* GetPoolsForWorld(UWorld* World);
	const TMap<TSubclassOf<AActor>, FActorPool>* GetPoolsForWorld(UWorld* World) const;

	/** 获取指定World的池配置（从DataAsset或默认） */
	FPoolConfig GetConfigForClass(TSubclassOf<AActor> ActorClass) const;
};
