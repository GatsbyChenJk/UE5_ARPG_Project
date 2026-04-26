// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterWeaponConfig.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UCharacterWeaponConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Weapon Config|Calculation")
	float AttackPower;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Weapon Config|Calculation")
	float DefensePower;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Weapon Config|Calculation")
	float AttackStaminaCost;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Weapon Config|Calculation")
	float DefenseStaminaCost;

	//TODO:Add weapon ability
};
