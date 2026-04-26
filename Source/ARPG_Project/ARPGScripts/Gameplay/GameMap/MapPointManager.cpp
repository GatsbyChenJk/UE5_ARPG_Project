// Fill out your copyright notice in the Description page of Project Settings.


#include "MapPointManager.h"

#include "EscapePointGenerator.h"
#include "SpawnPointGenerator.h"

void UMapPointManager::InitPointManager(const FMapManifest& InCurrentMapConfig)
{
	CurrentMapConfig = InCurrentMapConfig;
	InitAllPointGenerators();
}

FTransform UMapPointManager::GetRandomSpawnPointTransform(bool bIsSelfCamp)
{
	if (!SpawnPointGenerator) return FTransform::Identity;
	SpawnPointGenerator->SetPlayerCamp(bIsSelfCamp);
	return SpawnPointGenerator->GetRandomPointTransform();
}

FTransform UMapPointManager::GetRandomEscapePointTransform(float GameRunTime, bool bHasEscapeItem)
{
	if (!EscapePointGenerator) return FTransform::Identity;
	EscapePointGenerator->InitEscapeParams(GameRunTime, bHasEscapeItem);
	return EscapePointGenerator->GetRandomPointTransform();
}

void UMapPointManager::InitAllPointGenerators()
{
	if (!SpawnPointGenerator) SpawnPointGenerator = NewObject<USpawnPointGenerator>(this);
 	SpawnPointGenerator->InitGenerator(TArray<FBasePointData>(CurrentMapConfig.SpawnPoints));

 	if (!EscapePointGenerator) EscapePointGenerator = NewObject<UEscapePointGenerator>(this);
 	EscapePointGenerator->InitGenerator(TArray<FBasePointData>(CurrentMapConfig.EscapePoints));

	if (CurrentMapConfig.EscapePoints.Num() > 0)
	{
		EscapePointGenerator->SetEscapePointActor(CurrentMapConfig.EscapePoints[0].PortalClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MapPointManager::InitAllPointGenerators: EscapePoints array is empty!"));
	}
}
