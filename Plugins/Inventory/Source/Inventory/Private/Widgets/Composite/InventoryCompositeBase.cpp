// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Composite/InventoryCompositeBase.h"

void UInventoryCompositeBase::Collapse()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryCompositeBase::Expand()
{
	SetVisibility(ESlateVisibility::Visible);
}
