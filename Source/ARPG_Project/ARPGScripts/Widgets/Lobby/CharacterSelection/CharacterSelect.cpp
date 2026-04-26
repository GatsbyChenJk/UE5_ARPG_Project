// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterSelect.h"

#include "ARPGLeaf_CharacterChoice.h"
#include "ARPGLeaf_CharaterText.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/PlayerController/LobbyController.h"
#include "ARPGScripts/Widgets/Base/WidgetManager.h"
#include "Components/Button.h"

void UCharacterSelect::NativeConstruct()
{
	Super::NativeConstruct();

	if (const auto WidgetSubSystem = GetGameInstance()->GetSubsystem<UWidgetManager>())
	{
		WidgetSubSystem->RegisterWidget(FString("1002"),this);
	}
}

void UCharacterSelect::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (auto Child : GetChildren())
	{
		Child->ApplyFunction([this](UARPGCompositeBase* Child)
		{
			if (UARPGLeaf_CharacterChoice* Choice = Cast<UARPGLeaf_CharacterChoice>(Child))
			{
				Choice->SetParentWidget(this);
			}

			if (UARPGLeaf_CharacterText* DescText = Cast<UARPGLeaf_CharacterText>(Child))
			{
				DescText->SetParentWidget(this);
			}
		});
	}

	UPlayerGameInstance* PlayerGameInstance = Cast<UPlayerGameInstance>(GetGameInstance());
	if(IsValid(PlayerGameInstance))
	{
		PlayerGameInstance->LoadCharacterDataTableAsync();
	}

	Button_Confirm->OnClicked.AddDynamic(this,&ThisClass::OnConfirmCurrentChoice);
}

void UCharacterSelect::OnConfirmCurrentChoice()
{
	if (CurrentCharacterID.IsEmpty()) return;
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetWorld()->GetGameInstance()))
	{
		GameInstance->LoadCharacterDataFromTable(CurrentCharacterID);

		//serverRPC
		APlayerController* PlayerController = Cast<APlayerController>(GetOwningPlayer());
		if (ALobbyController* LobbyController = Cast<ALobbyController>(PlayerController))
		{
			LobbyController->Server_LoadCharacterDataFromTable(CurrentCharacterID);
		}
		UE_LOG(LogTemp,Warning,TEXT("CharacterData is set in PlayerGameInstance,where ID is %s"),*CurrentCharacterID);
	}
}
