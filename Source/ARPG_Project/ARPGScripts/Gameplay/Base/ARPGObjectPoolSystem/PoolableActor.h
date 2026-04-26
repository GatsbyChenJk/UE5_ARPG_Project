// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableActor.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPoolableActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可池化Actor接口
 * 实现此接口的Actor可以被PoolSubsystem管理，实现复用。
 *
 * 网络同步注意事项：
 * - 对象池在服务器端管理，Actor的bReplicates应设为true以同步到客户端
 * - OnActivateFromPool 只在服务器端调用，客户端通过复制看到Actor状态变化
 * - OnReturnToPool 只在服务器端调用，客户端看到Actor被隐藏/禁用
 * - 如果需要客户端预测或特殊处理，可在Actor自身中处理复制回调（如OnRep_*）
 */
class ARPG_PROJECT_API IPoolableActor
{
	GENERATED_BODY()

public:
	/**
	 * 当Actor从池中被取出并激活时调用（仅在服务器端调用）。
	 * 在此处恢复Actor的初始状态：重置生命值、填充弹药、重置动画等。
	 *
	 * @param Transform - Actor被放置的位置和旋转
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ObjectPool")
	void OnActivateFromPool(const FTransform& Transform);
	virtual void OnActivateFromPool_Implementation(const FTransform& Transform) {}

	/**
	 * 当Actor被释放回池中时调用（仅在服务器端调用）。
	 * 在此处清理Actor状态：停止特效、取消定时器、移除委托绑定等。
	 * 注意：碰撞、Tick、可见性等已由PoolSubsystem自动处理，无需在此重复设置。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ObjectPool")
	void OnReturnToPool();
	virtual void OnReturnToPool_Implementation() {}

	/**
	 * 检查此Actor当前是否正在使用中（已从池中取出）。
	 * 可在Blueprint中覆盖以实现自定义逻辑。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	bool IsActiveInPool() const;
	virtual bool IsActiveInPool_Implementation() const { return false; }
};
