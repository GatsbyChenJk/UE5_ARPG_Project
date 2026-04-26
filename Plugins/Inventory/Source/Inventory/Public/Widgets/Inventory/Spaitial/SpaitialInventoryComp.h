// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlottedItem/InventoryEquipSlottedItem.h"
#include "Widgets/Inventory/InventoryBase/InventoryBaseComp.h"
#include "SpaitialInventoryComp.generated.h"

struct FGameplayTag;
class UInventoryEquipmentSlot;
class UItemDescriptionWidget;
class UCanvasPanel;
class UButton;
class UWidgetSwitcher;
class UInventoryGrid;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMenuToggled, bool, bOpen);

UCLASS()
class INVENTORY_API USpaitialInventoryComp : public UInventoryBaseComp
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FInventorySlotAvailabilityResult HasRoomForItem(UItemComponent* ItemComp) const override;

	UItemDescriptionWidget* GetItemDescription();
	
	virtual void OnItemHovered(UInventoryItem* Item) override;
	virtual void OnItemUnHovered() override;
	virtual bool HasHoverItem() const override;
	virtual UInventoryHoverItem* GetHoverItem() const override;
	virtual float GetTileSize() const override;
	virtual void ToggleWidget(APlayerController* OwningController) override;

	void CharacterEquipAllWeapons();
	
	void DisableButton(UButton* Button);

	FInventoryMenuToggled OnInventoryMenuToggled;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGrid> Grid_Equippables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGrid> Grid_Consumables;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGrid> Grid_Craftables;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Equippable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consumable;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Craftable;
	
	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UItemDescriptionWidget> ItemDescWidgetClass;

	UPROPERTY()
	TObjectPtr<UItemDescriptionWidget> ItemDescWidget;

	FTimerHandle DescriptionTimer;

	UPROPERTY(EditAnywhere, Category="Inventory")
	float DescriptionTimerDelay = 0.5f;

	TArray<TObjectPtr<UInventoryEquipmentSlot>> EquippedGridSlots;

	void SwitchActiveGrid(UInventoryGrid* Grid,UButton* Button);

	UFUNCTION()
	void ShowEquippables();
	UFUNCTION()
	void ShowConsumables();
	UFUNCTION()
	void ShowCraftables();

	UFUNCTION()
	void EquippedSlottedItemClicked(UInventoryEquipSlottedItem* SlottedItem);
	
	UFUNCTION()
	void EquippedGridSlotClicked(UInventoryEquipmentSlot* EquippedGridSlot, const FGameplayTag& EquipmentTypeTag);

	void SetItemDescriptionSizeAndPosition(UItemDescriptionWidget* ItemDesc,UCanvasPanel* Canvas);

	bool CanEquipHoverItem(UInventoryEquipmentSlot* EquippedGridSlot, const FGameplayTag& EquipmentTypeTag) const;
	UInventoryEquipmentSlot* FindSlotWithEquippedItem(UInventoryItem* EquippedItem) const;
	void ClearSlotOfItem(UInventoryEquipmentSlot* EquippedGridSlot);
	void RemoveEquippedSlottedItem(UInventoryEquipSlottedItem* EquippedSlottedItem);
	void MakeEquippedSlottedItem(UInventoryEquipSlottedItem* EquippedSlottedItem, UInventoryEquipmentSlot* EquippedGridSlot, UInventoryItem* ItemToEquip);
	void BroadcastSlotClickedDelegates(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip) const;

	TWeakObjectPtr<UInventoryGrid> ActiveGrid;
};



