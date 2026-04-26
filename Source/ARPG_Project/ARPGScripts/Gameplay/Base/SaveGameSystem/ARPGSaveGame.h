// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ARPGSaveGame.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FARPGPlayerData
{
	GENERATED_BODY()
	// reflection macro is needed for load file
	// during serialization
	UPROPERTY()
	FString PlayerUserName;

	// shop data
	UPROPERTY()
	float PlayerTotalMoney;

	UPROPERTY()
	float HealthMod;
	
	UPROPERTY()
	float AttackPowerMod;

	UPROPERTY()
	float DefensePowerMod;

	void ResetMod()
	{
		HealthMod = 0;
		AttackPowerMod = 0;
		DefensePowerMod = 0;
	}
};

UCLASS()
class ARPG_PROJECT_API UARPGSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FARPGPlayerData PlayerData;

	UPROPERTY()
	int32 SlotIndex = 0;
};
