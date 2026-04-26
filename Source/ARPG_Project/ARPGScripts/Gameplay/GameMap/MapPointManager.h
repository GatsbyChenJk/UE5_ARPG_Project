// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMapManifest.h"
#include "UObject/Object.h"
#include "MapPointManager.generated.h"

class USpawnPointGenerator;
class UEscapePointGenerator;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UMapPointManager : public UObject
{
	GENERATED_BODY()
public:
	void InitPointManager(const FMapManifest& InCurrentMapConfig);
	FTransform GetRandomSpawnPointTransform(bool bIsSelfCamp = true);
	FTransform GetRandomEscapePointTransform(float GameRunTime = 10.f, bool bHasEscapeItem = false);

	UFUNCTION()
	UEscapePointGenerator* GetEscapePointGenerator()
	{
		return EscapePointGenerator;
	};
private:
	FMapManifest CurrentMapConfig;

	UPROPERTY()
	USpawnPointGenerator* SpawnPointGenerator;

	UPROPERTY()
	UEscapePointGenerator* EscapePointGenerator;
	
	void InitAllPointGenerators();
};
