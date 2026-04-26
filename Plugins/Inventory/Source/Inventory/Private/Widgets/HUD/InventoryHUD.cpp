// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/InventoryHUD.h"

#include "Inventory/Component/InventoryComp.h"
#include "Inventory/Util/InventoryStatics.h"
#include "Widgets/HUD/InfoMessage.h"

void UInventoryHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UInventoryComp* InventoryComp = UInventoryStatics::GetInventoryComp(GetOwningPlayer());
	if (IsValid(InventoryComp))
	{
		InventoryComp->OnNoRoomInInventory.AddDynamic(this,&ThisClass::OnNoRoom);
	}

	InfoMessage->MessageHide();
}

void UInventoryHUD::OnCloseWidget(APlayerController* OwningController)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	bIsWidgetOpen = false;
}

void UInventoryHUD::OnOpenWidget(APlayerController* OwningController)
{
	SetVisibility(ESlateVisibility::Hidden);
	bIsWidgetOpen = true;
}

void UInventoryHUD::OnNoRoom()
{
	if (!IsValid(InfoMessage)) return;
	InfoMessage->SetMessage(FText::FromString("The Inventory is full"));
}
