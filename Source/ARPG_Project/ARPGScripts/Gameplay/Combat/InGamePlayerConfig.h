// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InGamePlayerConfig.generated.h"

struct FGameplayTag;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FSpawnCharacterDataID
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString DefaultCharacterID;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ID | Special Operations")
	FString SprintAbilityID;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ID | Special Operations")
	FString RollAbilityID;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ID | Abilities")
	FString AttackAbilityID;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ID | Abilities")
	FString DefenseAbilityID;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Attributes | Buff")
	TArray<FGameplayTag> BuffTags;
};

UCLASS(BlueprintType)
class ARPG_PROJECT_API UInGamePlayerConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FSpawnCharacterDataID CharacterIDConfigs;
};
