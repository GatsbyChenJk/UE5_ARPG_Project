// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Util/InventoryStatics.h"

#include "AIController.h"
#include "Inventory/Component/InventoryComp.h"
#include "Items/Components/ItemComponent.h"
#include "Types/InventoryGridType.h"
#include "Widgets/Inventory/InventoryBase/InventoryBaseComp.h"

UInventoryComp* UInventoryStatics::GetInventoryComp(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController)) return nullptr;

	UInventoryComp* InventoryComp = PlayerController->FindComponentByClass<UInventoryComp>();

	return InventoryComp;
}

UInventoryComp* UInventoryStatics::GetAIInventoryComp(const AAIController* AIController)
{
	if (!IsValid(AIController)) return nullptr;

	UInventoryComp* InventoryComp = AIController->FindComponentByClass<UInventoryComp>();

	return InventoryComp;
}

EInventoryGridType UInventoryStatics::GetItemCategoryFromItemComp(const UItemComponent* ItemComp)
{
	if (!IsValid(ItemComp)) return EInventoryGridType::None;

	return ItemComp->GetItemManifest().GetItemCategory();
}

void UInventoryStatics::ItemHovered(APlayerController* PC, UInventoryItem* Item)
{
	UInventoryComp* IC = GetInventoryComp(PC);
	if (!IsValid(IC)) return;

	UInventoryBaseComp* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return;

	if (InventoryBase->HasHoverItem()) return;

	InventoryBase->OnItemHovered(Item);
}

void UInventoryStatics::ItemUnhovered(APlayerController* PC)
{
	UInventoryComp* IC = GetInventoryComp(PC);
	if (!IsValid(IC)) return;

	UInventoryBaseComp* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return;

	InventoryBase->OnItemUnHovered();
}

UInventoryHoverItem* UInventoryStatics::GetHoverItem(APlayerController* PC)
{
	UInventoryComp* IC = GetInventoryComp(PC);
	if (!IsValid(IC)) return nullptr;

	UInventoryBaseComp* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return nullptr;

	return InventoryBase->GetHoverItem();
}

UInventoryBaseComp* UInventoryStatics::GetInventoryWidget(APlayerController* PC)
{
	UInventoryComp* IC = GetInventoryComp(PC);
	if (!IsValid(IC)) return nullptr;

	return IC->GetInventoryMenu();
}
