// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../../../../Source/ARPG_Project//ARPGScripts/Widgets/Base/BaseWidget.h"
#include "Types/InventoryGridType.h"
#include "InventoryBaseComp.generated.h"

class UInventoryHoverItem;
class UItemComponent;
class UInventoryItem;

UCLASS()
class INVENTORY_API UInventoryBaseComp : public UBaseWidget
{
	GENERATED_BODY()
public:
	virtual FInventorySlotAvailabilityResult HasRoomForItem(UItemComponent* ItemComp) const { return FInventorySlotAvailabilityResult();}	
	virtual void OnItemHovered(UInventoryItem* Item) {}
	virtual void OnItemUnHovered() {}
	virtual bool HasHoverItem() const { return false; }
	virtual UInventoryHoverItem* GetHoverItem() const { return nullptr; }
	virtual float GetTileSize() const { return 0.0f; }
};
