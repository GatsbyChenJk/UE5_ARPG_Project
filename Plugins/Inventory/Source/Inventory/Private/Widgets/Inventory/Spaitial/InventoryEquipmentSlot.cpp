// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spaitial/InventoryEquipmentSlot.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Inventory/InventoryItem.h"
#include "Inventory/Util/InventoryStatics.h"
#include "Items/Fragment/ItemFragment.h"
#include "Items/Fragment/ItemFragmentTag.h"
#include "Widgets/Inventory/HoverItems/InventoryHoverItem.h"
#include "Widgets/Inventory/Spaitial/SlottedItem/InventoryEquipSlottedItem.h"

void UInventoryEquipmentSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!GetAvailable()) return;
	UInventoryHoverItem* HoverItem = UInventoryStatics::GetHoverItem(GetOwningPlayer());
	if (!IsValid(HoverItem)) return;

	if (HoverItem->GetItemTag().MatchesTag(EquipmentTypeTag))
	{
		SetOccupiedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventoryEquipmentSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (!GetAvailable()) return;
	UInventoryHoverItem* HoverItem = UInventoryStatics::GetHoverItem(GetOwningPlayer());
	if (!IsValid(HoverItem)) return;

	if (IsValid(EquippedSlottedItem)) return;

	if (HoverItem->GetItemTag().MatchesTag(EquipmentTypeTag))
	{
		SetUnoccupiedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

FReply UInventoryEquipmentSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	EquippedGridSlotClicked.Broadcast(this, EquipmentTypeTag);
	return FReply::Handled();
}

UInventoryEquipSlottedItem* UInventoryEquipmentSlot::OnItemEquipped(UInventoryItem* Item,
	const FGameplayTag& EquipmentTag, float TileSize)
{
	if (!EquipmentTag.MatchesTagExact(EquipmentTypeTag)) return nullptr;
	
	// Get Grid Dimensions
	const FGridFragment* GridFragment = GetFragment<FGridFragment>(Item, FragmentTag::GridFragment);
	if (!GridFragment) return nullptr;
	const FIntPoint GridDimensions = GridFragment->GetGridSize();

	// Calculate the Draw Size for the Equipped Slotted Item
	const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	const FVector2D DrawSize = GridDimensions * IconTileWidth;

	// Create the Equipped Slotted Item widget
	EquippedSlottedItem = CreateWidget<UInventoryEquipSlottedItem>(GetOwningPlayer(), EquippedSlottedItemClass);
	
	// Set the Slotted Item's Inventory Item
	EquippedSlottedItem->SetInventoryItem(Item);
	
	// Set the Slotted Item's Equipment Type Tag
	EquippedSlottedItem->SetEquipmentTypeTag(EquipmentTag);
	
	// Hide the Stack Count widget on the Slotted Item
	EquippedSlottedItem->UpdateStackCount(0);
	
	// Set Inventory Item on this class (the Equipped Grid Slot)
	SetInventoryItem(Item);

	// Set the Image Brush on the Equipped Slotted Item
	const FImageFragment* ImageFragment = GetFragment<FImageFragment>(Item, FragmentTag::ImageFragment);
	if (!ImageFragment) return nullptr;

	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = DrawSize;
	
	EquippedSlottedItem->SetImageBrush(Brush);
	
	// Add the Slotted Item as a child to this widget's Overlay
	Overlay_Root->AddChildToOverlay(EquippedSlottedItem);
	FGeometry OverlayGeometry = Overlay_Root->GetCachedGeometry();
	auto OverlayPos = OverlayGeometry.Position;
	auto OverlaySize = OverlayGeometry.Size;

	const float LeftPadding = OverlaySize.X / 2.f - DrawSize.X / 2.f;
	const float TopPadding = OverlaySize.Y / 2.f - DrawSize.Y / 2.f;

	UOverlaySlot* OverlaySlot = UWidgetLayoutLibrary::SlotAsOverlaySlot(EquippedSlottedItem);
	OverlaySlot->SetPadding(FMargin(LeftPadding, TopPadding));
	
	return EquippedSlottedItem;
}
