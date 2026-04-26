// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spaitial/SlottedItem/InventoryEquipSlottedItem.h"

FReply UInventoryEquipSlottedItem::NativeOnMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	OnEquippedSlottedItemClicked.Broadcast(this);
	return FReply::Handled();
}
