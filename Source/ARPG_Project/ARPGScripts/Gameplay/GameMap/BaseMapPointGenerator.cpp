// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMapPointGenerator.h"
#include "FMapManifest.h"
#include "Kismet/KismetMathLibrary.h"

TArray<FBasePointData> UBaseMapPointGenerator::GetValidPoints_Implementation()
{
	TArray<FBasePointData> ValidPoints;
	for (const FBasePointData& Point : AllPoints)
	{
		if (Point.bIsEnabled)
		{
			ValidPoints.Add(Point);
		}
	}
	return ValidPoints;
}

FTransform UBaseMapPointGenerator::GetRandomPointTransform()
{
	TArray<FBasePointData> ValidPoints = GetValidPoints();
	if (ValidPoints.Num() <= 0)
	{
		return FTransform::Identity;
	}

	int32 RandomIndex = GetRandomIndexByWeight(ValidPoints);
	const FBasePointData& SelectedPoint = ValidPoints[RandomIndex];

	FTransform PointTransform;
	PointTransform.SetLocation(SelectedPoint.PointTransform.GetLocation());
	PointTransform.SetRotation(SelectedPoint.PointTransform.GetRotation());
	PointTransform.SetScale3D(FVector::OneVector);

	return PointTransform;
}

TArray<FTransform> UBaseMapPointGenerator::GetRandomPointTransforms(int32 Num)
{
	TArray<FTransform> ResultTransforms;
	TArray<FBasePointData> ValidPoints = GetValidPoints();
	if (ValidPoints.Num() <= 0 || Num <= 0)
	{
		return ResultTransforms;
	}

	for (int32 i = 0; i < Num; i++)
	{
		if (ValidPoints.Num() <= 0) break;
		int32 RandomIndex = GetRandomIndexByWeight(ValidPoints);
		const FBasePointData& SelectedPoint = ValidPoints[RandomIndex];
                
		FTransform PointTransform;
		PointTransform.SetLocation(SelectedPoint.PointTransform.GetLocation());
		PointTransform.SetRotation(SelectedPoint.PointTransform.GetRotation());
		PointTransform.SetScale3D(FVector::OneVector);

		ResultTransforms.Add(PointTransform);
		ValidPoints.RemoveAt(RandomIndex); // 去重：选过的点位不再选
	}

	return ResultTransforms;
}

int32 UBaseMapPointGenerator::GetRandomIndexByWeight(const TArray<FBasePointData>& ValidPoints)
{
	int32 TotalWeight = 0;
	for (const FBasePointData& Point : ValidPoints)
	{
		TotalWeight += FMath::Max(Point.SelectWeight, 1);
	}

	int32 RandomWeight = UKismetMathLibrary::RandomIntegerInRange(1, TotalWeight);
	int32 CurrentWeight = 0;
	for (int32 i = 0; i < ValidPoints.Num(); i++)
	{
		CurrentWeight += FMath::Max(ValidPoints[i].SelectWeight, 1);
		if (RandomWeight <= CurrentWeight)
		{
			return i;
		}
	}

	return 0;
}
