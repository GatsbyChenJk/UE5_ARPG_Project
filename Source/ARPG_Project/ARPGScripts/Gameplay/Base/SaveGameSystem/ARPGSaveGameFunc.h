// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ARPGSaveGameFunc.generated.h"

class UARPGSaveGame;
class AARPGBaseCharacter;
// This class does not need to be modified.
UINTERFACE()
class UARPGSaveGameFunc : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPG_PROJECT_API IARPGSaveGameFunc
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintNativeEvent, Category = "SaveGameSave")
	void SaveCharacterData(AARPGBaseCharacter* Player);
};
