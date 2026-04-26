// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventoryGridSlot.h"
#include "InventoryEquipmentSlot.generated.h"

class UOverlay;
class UInventoryEquipSlottedItem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquippedGridSlotClicked, UInventoryEquipmentSlot*, GridSlot,
                                             const FGameplayTag&, EquipmentTypeTag);

/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryEquipmentSlot : public UInventoryGridSlot
{
	GENERATED_BODY()
public:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UInventoryEquipSlottedItem* OnItemEquipped(UInventoryItem* Item, const FGameplayTag& EquipmentTag, float TileSize);
	void SetEquippedSlottedItem(UInventoryEquipSlottedItem* Item) { EquippedSlottedItem = Item; }
	
	FEquippedGridSlotClicked EquippedGridSlotClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UInventoryEquipSlottedItem> EquippedSlottedItemClass;

	UPROPERTY()
	TObjectPtr<UInventoryEquipSlottedItem> EquippedSlottedItem;
private:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "GameItems.Equipment"))
	FGameplayTag EquipmentTypeTag;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_GrayedOutIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_Root;
};
