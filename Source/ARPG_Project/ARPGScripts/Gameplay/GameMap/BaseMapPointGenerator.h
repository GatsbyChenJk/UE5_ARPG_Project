// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FMapManifest.h"
#include "BaseMapPointGenerator.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable,BlueprintType)
class ARPG_PROJECT_API UBaseMapPointGenerator : public UObject
{
	GENERATED_BODY()
public:
	virtual void InitGenerator(const TArray<FBasePointData>& InAllPoints) { AllPoints = InAllPoints; }
	
	UFUNCTION(BlueprintNativeEvent, Category = "PointGenerator")
	TArray<FBasePointData> GetValidPoints();
	
	virtual TArray<FBasePointData> GetValidPoints_Implementation();
	
	UFUNCTION(BlueprintCallable, Category = "PointGenerator")
	FTransform GetRandomPointTransform();
	
	UFUNCTION(BlueprintCallable, Category = "PointGenerator")
	TArray<FTransform> GetRandomPointTransforms(int32 Num);

protected:
	// 当前地图的所有该类型点位
	TArray<FBasePointData> AllPoints;

	// 通用工具：根据权重随机选一个索引（权重越高，概率越大）
	int32 GetRandomIndexByWeight(const TArray<FBasePointData>& ValidPoints);
};
