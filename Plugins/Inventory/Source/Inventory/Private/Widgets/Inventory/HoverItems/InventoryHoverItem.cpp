// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/HoverItems/InventoryHoverItem.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/InventoryItem.h"

void UInventoryHoverItem::SetImageBrush(const FSlateBrush& Brush)
{
	Image_Icon->SetBrush(Brush);
}

void UInventoryHoverItem::UpdateStackCount(const int32 count)
{
	StackCount = count;
	if (count > 0)
	{
		Text_StackCount->SetText(FText::AsNumber(count));
		Text_StackCount->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FGameplayTag UInventoryHoverItem::GetItemTag() const
{
	if (InventoryItem.IsValid())
	{
		return  InventoryItem->GetItemManifest().GetGameplayTag();
	}
	return FGameplayTag();
}

void UInventoryHoverItem::SetIsStackable(const bool bStackable)
{
	bIsStackable = bStackable;
	if (!bIsStackable)
	{
		Text_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UInventoryItem* UInventoryHoverItem::GetInventoryItem() const
{
	return InventoryItem.Get();
}

void UInventoryHoverItem::SetInventoryItem(UInventoryItem* Item)
{
	InventoryItem = Item;
}
