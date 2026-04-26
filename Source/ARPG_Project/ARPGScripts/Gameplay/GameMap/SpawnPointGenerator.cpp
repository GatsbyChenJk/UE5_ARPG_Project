// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnPointGenerator.h"

#include "FMapManifest.h"

TArray<FBasePointData> USpawnPointGenerator::GetValidPoints_Implementation()
{
	TArray<FBasePointData> ValidPoints;
	for (const FBasePointData& BasePoint : AllPoints)
	{
		const FSpawnPointData& SpawnPoint = static_cast<const FSpawnPointData&>(BasePoint);
		if (!SpawnPoint.bIsEnabled) continue;
		if (SpawnPoint.bIsSelfCamp && !bCurrentSelfCamp) continue;

		ValidPoints.Add(SpawnPoint);
	}
	return ValidPoints;
}

