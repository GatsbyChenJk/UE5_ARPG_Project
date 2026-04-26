// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ObjectPoolConfig.generated.h"

/**
 * 单个Actor池的配置结构
 */
USTRUCT(BlueprintType)
struct FPoolConfig
{
	GENERATED_BODY()

	/** 池的初始预生成数量（加载时即创建） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool", meta = (ClampMin = "0"))
	int32 InitialSize = 0;

	/** 池的最大容量（0 表示无限制） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool", meta = (ClampMin = "0"))
	int32 MaxSize = 100;

	/** 是否启用池化（若 false 则回退到普通 Spawn/Destroy） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	bool bEnablePooling = true;

	/** 是否在游戏开始时自动预热（生成InitialSize数量的Actor） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool", meta = (EditCondition = "bEnablePooling"))
	bool bAutoPrewarm = false;

	FPoolConfig() {}
};

/**
 * 对象池配置文件
 * 在DataAsset中为每种需要池化的Actor类配置参数
 */
UCLASS(BlueprintType)
class ARPG_PROJECT_API UObjectPoolConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 每个 Actor 类的池配置映射 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObjectPool", meta = (ForceInlineRow))
	TMap<TSubclassOf<AActor>, FPoolConfig> PoolConfigs;

	/**
	 * 获取指定类的配置，若不存在则返回默认配置
	 * @param ActorClass - 需要查询配置的Actor类
	 * @return 对应的池配置，未配置时返回默认值
	 */
	UFUNCTION(BlueprintPure, Category = "ObjectPool")
	FPoolConfig GetConfigForClass(TSubclassOf<AActor> ActorClass) const
	{
		if (const FPoolConfig* Found = PoolConfigs.Find(ActorClass))
		{
			return *Found;
		}
		// 返回默认配置（允许启用池，但大小由代码决定）
		return FPoolConfig();
	}

	/**
	 * 检查指定类是否配置了池化
	 */
	UFUNCTION(BlueprintPure, Category = "ObjectPool")
	bool HasConfigForClass(TSubclassOf<AActor> ActorClass) const
	{
		return PoolConfigs.Contains(ActorClass);
	}

	/**
	 * 获取所有配置了自动预热的类
	 */
	UFUNCTION(BlueprintPure, Category = "ObjectPool")
	TArray<TSubclassOf<AActor>> GetAutoPrewarmClasses() const
	{
		TArray<TSubclassOf<AActor>> Result;
		for (const auto& Pair : PoolConfigs)
		{
			if (Pair.Value.bEnablePooling && Pair.Value.bAutoPrewarm && Pair.Value.InitialSize > 0)
			{
				Result.Add(Pair.Key);
			}
		}
		return Result;
	}
};
