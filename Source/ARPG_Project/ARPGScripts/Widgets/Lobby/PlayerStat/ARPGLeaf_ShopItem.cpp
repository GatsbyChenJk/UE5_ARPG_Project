// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGLeaf_ShopItem.h"

#include "ARPGLobbyPlayerStat.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Character/Lobby/ShoppingSystem/ShopItemManifest.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UARPGLeaf_ShopItem::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(Image_ItemIcon))
	{
		Image_ItemIcon->SetBrushFromTexture(Texture_ShopItemIcon);
	}

	if (IsValid(Text_ShopItemName))
	{
		Text_ShopItemName->SetText(FText::FromString(ShopItemName));
	}
	
}

void UARPGLeaf_ShopItem::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Select->OnClicked.AddDynamic(this,&ThisClass::OnSelectShopItem);
}

void UARPGLeaf_ShopItem::SetParentWidget(UARPGLobbyPlayerStat* InParent)
{
	ParentWidget = InParent;
}

void UARPGLeaf_ShopItem::OnSelectShopItem()
{
	// set parent description text
	// set effect value
	if (IsValid(ParentWidget))
	{
		if (auto GI = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			FShopItemManifest ShopItemManifest = GI->GetShopItemManifestByID(ShopItemID);

			ParentWidget->SetCurrentMoneyToPay(ShopItemManifest.ItemPrice);
			ParentWidget->SetCurrentAttackPowerUp(ShopItemManifest.AttackPowerUp);
			ParentWidget->SetCurrentDefensePowerUp(ShopItemManifest.DefensePowerUp);
			ParentWidget->SetCurrentHealthPowerUp(ShopItemManifest.HealthPowerUp);
			ParentWidget->SetShopItemDescriptionText(FText::FromString(ShopItemManifest.ItemDesc));
		}
		else
		{
			ParentWidget->SetCurrentMoneyToPay(0.0f);
			ParentWidget->SetCurrentAttackPowerUp(0.0f);
			ParentWidget->SetCurrentDefensePowerUp(0.0f);
			ParentWidget->SetCurrentHealthPowerUp(0.0f);
			ParentWidget->SetShopItemDescriptionText(FText::FromString("no effect"));
		}
	}
}
