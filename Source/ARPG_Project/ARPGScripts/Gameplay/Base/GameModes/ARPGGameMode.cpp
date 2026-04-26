// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGGameMode.h"

#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"

void AARPGGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GameInstance->LoadCharacterDataTableAsync();
		GameInstance->LoadLevelConfigDataTableAsync();
		GameInstance->LoadWeaponDataTableAsync();
		GameInstance->LoadWidgetDataTableAsync();
		GameInstance->LoadOperationDataTableAsync();
		GameInstance->LoadShopSystemDataTableAsync();
	}
}

