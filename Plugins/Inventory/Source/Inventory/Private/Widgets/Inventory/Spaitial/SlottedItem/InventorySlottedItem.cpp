// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spaitial/SlottedItem/InventorySlottedItem.h"
#include "Inventory/InventoryItem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/Util/InventoryStatics.h"

FReply UInventorySlottedItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnSlottedItemClicked.Broadcast(GridIndex,InMouseEvent);
	return FReply::Handled();
}

void UInventorySlottedItem::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UInventoryStatics::ItemHovered(GetOwningPlayer(),InventoryItem.Get());
}

void UInventorySlottedItem::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	UInventoryStatics::ItemUnhovered(GetOwningPlayer());
}

void UInventorySlottedItem::SetInventoryItem(UInventoryItem* Item)
{
	 InventoryItem = Item; 
}

UInventoryItem* UInventorySlottedItem::GetInventoryItem() const
{
	return InventoryItem.Get();
}

void UInventorySlottedItem::SetImageBrush(const FSlateBrush& Brush) const
{
	Image_Icon->SetBrush(Brush);
}

void UInventorySlottedItem::UpdateStackCount(int32 StackCount)
{
	if (StackCount > 0)
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
		Text_StackCount->SetText(FText::AsNumber(StackCount));
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}
