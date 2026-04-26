// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spaitial/InventoryGridSlot.h"

#include "Components/Image.h"
#include "Widgets/Inventory/PopUpMenu/ItemPopUp.h"

void UInventoryGridSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	GridSlotHovered.Broadcast(TileIndex,InMouseEvent);
}

void UInventoryGridSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	GridSlotUnhovered.Broadcast(TileIndex, InMouseEvent);
}

FReply UInventoryGridSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	GridSlotClicked.Broadcast(TileIndex, InMouseEvent);
	
	return FReply::Handled();
}

void UInventoryGridSlot::SetInventoryItem(UInventoryItem* Item)
{
	InventoryItem = Item;
}

void UInventoryGridSlot::SetItemPopup(UItemPopUp* InPopup)
{
	ItemPopUp = InPopup;
	ItemPopUp->SetGridIndex(GetTileIndex());
	ItemPopUp->OnNativeDestruct.AddUObject(this,&ThisClass::OnItemPopupDestruct);
}

void UInventoryGridSlot::SetOccupiedTexture()
{
	GridSlotState = EInventoryGridSlotState::Occupied;
	Image_GridSlot->SetBrush(Brush_Occupied);
}

void UInventoryGridSlot::SetUnoccupiedTexture()
{
	GridSlotState = EInventoryGridSlotState::Unoccupied;
	Image_GridSlot->SetBrush(Brush_UnOccupied);
}

void UInventoryGridSlot::SetSelectedTexture()
{
	GridSlotState = EInventoryGridSlotState::Selected;
	Image_GridSlot->SetBrush(Brush_Selected);
}

void UInventoryGridSlot::SetGrayedOutTexture()
{
	GridSlotState = EInventoryGridSlotState::GrayedOut;
	Image_GridSlot->SetBrush(Brush_GrayOut);
}

void UInventoryGridSlot::OnItemPopupDestruct(UUserWidget* Menu)
{
	ItemPopUp.Reset();
}
