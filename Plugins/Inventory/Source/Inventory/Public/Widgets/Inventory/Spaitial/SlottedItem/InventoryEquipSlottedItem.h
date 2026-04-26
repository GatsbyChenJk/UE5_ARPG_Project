// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InventorySlottedItem.h"
#include "InventoryEquipSlottedItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquippedSlottedItemClicked, class UInventoryEquipSlottedItem*, SlottedItem);

/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryEquipSlottedItem : public UInventorySlottedItem
{
	GENERATED_BODY()
public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	void SetEquipmentTypeTag(const FGameplayTag& Tag) { EquipmentTypeTag = Tag; }
	FGameplayTag GetEquipmentTypeTag() const { return EquipmentTypeTag; }

	FEquippedSlottedItemClicked OnEquippedSlottedItemClicked;
private:

	UPROPERTY()
	FGameplayTag EquipmentTypeTag;
};
