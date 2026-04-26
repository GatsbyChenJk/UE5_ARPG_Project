// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGLobbyPlayerStat.h"

#include "ARPGLeaf_ShopItem.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Base/SaveGameSystem/ARPGSaveGame.h"
#include "ARPGScripts/Gameplay/Base/SaveGameSystem/ARPGSaveGameManager.h"
#include "ARPGScripts/Gameplay/PlayerController/LobbyController.h"
#include "ARPGScripts/Widgets/InGame/HUDWidgets/ARPGLeaf_IconText.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UARPGLobbyPlayerStat::LoadCharacterMoneyData()
{
	if (auto PC = Cast<ALobbyController>(GetOwningPlayer()))
	{
		PC->LoadSavedGame();
	}
}

void UARPGLobbyPlayerStat::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (auto Child : GetChildren())
	{
		if (auto ShopItem = Cast<UARPGLeaf_ShopItem>(Child))
		{
			ShopItem->SetParentWidget(this);
		}
	}

	// load shop item config
	if (auto PlayerGI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		PlayerGI->LoadShopSystemDataTableAsync();
	}

	// load and set character data
	if (const auto SaveGameMgr = GetGameInstance()->GetSubsystem<UARPGSaveGameManager>())
	{
		SaveGameMgr->OnLoadCompleted.AddDynamic(this,&ThisClass::OnCharacterDataLoadComplete);
	}

	LoadCharacterMoneyData();

	Button_Purchase->OnClicked.AddDynamic(this,&ThisClass::OnCharacterPurchase);
}

void UARPGLobbyPlayerStat::SetShopItemDescriptionText(FText Desc)
{
	Text_ItemDescription->SetText(Desc);
}

void UARPGLobbyPlayerStat::SetCurrentAttackPowerUp(float Value)
{
	CurrentItemAttackPower = Value;
}

void UARPGLobbyPlayerStat::SetCurrentDefensePowerUp(float Value)
{
	CurrentItemDefensePower = Value;
}

void UARPGLobbyPlayerStat::SetCurrentHealthPowerUp(float Value)
{
	CurrentItemHealthMod = Value;
}

void UARPGLobbyPlayerStat::SetCurrentMoneyToPay(float Value)
{
	CurrentShopItemMoney = Value;
	Text_ItemPrice->SetText(FText::AsNumber(Value));
}

void UARPGLobbyPlayerStat::SetMoneyValueText(float MoneyVal)
{
	if (auto MoneyWidget = Cast<UARPGLeaf_IconText>(FindChildByTag(MoneyTag)))
	{
		MoneyWidget->SetValueText(MoneyVal);
	}
}

void UARPGLobbyPlayerStat::OnCharacterDataLoadComplete(bool bSuccess, int32 SlotIndex, UARPGSaveGame* SaveGame)
{
	if (IsValid(SaveGame))
	{
		// Find IconText in child,set text value in SaveGame
		auto CurrentPlayerData = SaveGame->PlayerData;
		float MoneyVal = CurrentPlayerData.PlayerTotalMoney;

		SetMoneyValueText(MoneyVal);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1,3.0f,FColor::Red,TEXT("Load Player SaveGame Data Failed!"));
	}
}

void UARPGLobbyPlayerStat::OnCharacterPurchase()
{
	// TODO: if current money is not enough , show tip ui
	if (CurrentMoney < CurrentShopItemMoney)
	{
		return;
	}
	// 1 update money on UI
	CurrentMoney -= CurrentShopItemMoney;
	SetMoneyValueText(CurrentMoney);
	// 2.1 update money in savegame
	// 2.2 update effect data in savegame
	FARPGPlayerData UpdatedPlayerData;
	UpdatedPlayerData.PlayerTotalMoney = CurrentMoney;
	UpdatedPlayerData.AttackPowerMod = CurrentItemAttackPower;
	UpdatedPlayerData.DefensePowerMod = CurrentItemDefensePower;
	UpdatedPlayerData.HealthMod = CurrentItemHealthMod;

	if (auto SaveMgr = GetGameInstance()->GetSubsystem<UARPGSaveGameManager>())
	{
		SaveMgr->AsyncSaveGame(UpdatedPlayerData, 0);
	}
}
