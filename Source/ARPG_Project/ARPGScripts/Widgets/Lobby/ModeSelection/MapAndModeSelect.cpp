// Fill out your copyright notice in the Description page of Project Settings.


#include "MapAndModeSelect.h"

#include "ARPGLeaf_MapChoice.h"
#include "ARPGLeaf_ModeChoice.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/PlayerController/LobbyController.h"
#include "Components/Button.h"

void UMapAndModeSelect::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (auto Child : GetChildren())
	{
		// set leaf parent widget
		Child->ApplyFunction([this](UARPGCompositeBase* Child)
		{
			if (UARPGLeaf_MapChoice* MapChoice = Cast<UARPGLeaf_MapChoice>(Child))
			{
				MapChoice->SetParentWidget(this);
			}

			if (UARPGLeaf_ModeChoice* ModeChoice = Cast<UARPGLeaf_ModeChoice>(Child))
			{
				ModeChoice->SetParentWidget(this);
			}
		});
	}

	// load in client
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetWorld()->GetGameInstance()))
	{
		GameInstance->LoadLevelConfigDataTableAsync();
	}

	Button_Confirm->OnClicked.AddDynamic(this,&ThisClass::OnConfirmCurrentChoice);
}

void UMapAndModeSelect::OnConfirmCurrentChoice()
{
	if (CurrentMapID.IsEmpty()) return;
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetWorld()->GetGameInstance()))
	{
		GameInstance->LoadLevelConfigDataFromTable(CurrentMapID);
	}
	
	// server part execute in controller
	APlayerController* PlayerController = GetOwningPlayer();
	if (ALobbyController* LobbyController = Cast<ALobbyController>(PlayerController))
	{
		LobbyController->Server_LoadLevelConfigDataFromTable(CurrentMapID);
	}
}
