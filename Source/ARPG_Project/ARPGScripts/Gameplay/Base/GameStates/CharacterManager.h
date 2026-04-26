// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CharacterManager.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UCharacterManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void RegisterCharacter(ACharacter* Character);

	UFUNCTION(BlueprintCallable)
	void UnRegisterCharacter(ACharacter* Character);

	ACharacter* GetPlayerByIndex(int32 CharIndex);

	TArray<ACharacter*> GetAllCharacter() { return RegisteredCharacters;};
private:
	TArray<ACharacter*> RegisteredCharacters;
};
