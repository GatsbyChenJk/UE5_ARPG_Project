// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryGridSlot.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryItem.h"
#include "Items/Fragment/ItemFragment.h"
#include "Types/InventoryGridType.h"
#include "InventoryGrid.generated.h"

class UItemPopUp;
class UInventoryHoverItem;
struct FGridFragment;
class UInventorySlottedItem;
struct FItemManifest;
class UItemComponent;
class UInventoryComp;
class UCanvasPanel;
class UInventoryGridSlot;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryGrid : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	EInventoryGridType GetGridType() const { return GridType; }

	FInventorySlotAvailabilityResult HasRoomForItem(const UItemComponent* ItemComp);
	
	void DropItem();
	bool HasHoverItem() const;
	UInventoryHoverItem* GetHoverItem();
	float GetTileSize() const {return TileSize; };
	void ClearHoverItem();
	void AssignHoverItem(UInventoryItem* InventoryItem);
	void OnHide();
	
	UFUNCTION()
	void AddItem(UInventoryItem* Item);
	
	UFUNCTION()
	void AddStacks(const FInventorySlotAvailabilityResult& Result);

	UFUNCTION()
	void OnSlottedItemClicked(int32 GridIndex,const FPointerEvent& InMouseEvent);

	UFUNCTION()
	void OnGridSlotClicked(int32 GridIndex,const FPointerEvent& InMouseEvent);

	UFUNCTION()
	void OnGridSlotHovered(int32 GridIndex,const FPointerEvent& InMouseEvent);

	UFUNCTION()
	void OnGridSlotUnhovered(int32 GridIndex,const FPointerEvent& InMouseEvent);

	UFUNCTION()
	void OnPopUpMenuSplit(int SplitAmount, int Index);
	
	UFUNCTION()
	void OnPopUpMenuDrop(int Index);

	UFUNCTION()
	void OnPopUpMenuConsume(int Index);

	UFUNCTION()
	void OnPopUpMenuSell(int Index);

	void SetOwningCanvasPanel(UCanvasPanel* NewCanvasPanel);
private:
	void ConstructGrid();
	
	FInventorySlotAvailabilityResult HasRoomForItem(const UInventoryItem* Item,const int32 StackAmountOverride = -1);
	
	FInventorySlotAvailabilityResult HasRoomForItem(const FItemManifest& ItemManifest,const int32 StackAmountOverride = -1);

	// add item icon widget start
	void SetSlateItemImage(const FGridFragment* GridFragment, const FImageFragment* ImageFragment,const UInventorySlottedItem* SlottedItem) const;
	void AddItemToIndices(const FInventorySlotAvailabilityResult& Result,UInventoryItem* Item);
	void AddItemAtIndex(UInventoryItem* Item,const int32 Index,const bool bStackable,const int32 StackCount);
	bool MatchesCategory(const UInventoryItem* Item) const;
	FVector2D GetDrawSize(const FGridFragment* GridFragment) const;
	UInventorySlottedItem* CreateSlottedItem(
		UInventoryItem* Item,
		const bool bStackable,
		const int32 StackAmount,
		const FGridFragment* GridFragment,
		const FImageFragment* ImageFragment,
		const int32 Index
		) const;
	void AddSlottedItemToCanvas(const int32 Index,const FGridFragment* GridFragment,UInventorySlottedItem* SlottedItem) const;
	void UpdateGridSlot(UInventoryItem* Item,const int32 Index,bool bStackableItem,const int32 StackAmount);
	// add item icon widget end

	// item check and set start
	int32 GetStackAmount(const UInventoryGridSlot* GridSlot) const;
	int32 DetermineFillAmoundForSlot(const bool bStackable,const int32 MaxStackSize,const int32 AmountToFill,const UInventoryGridSlot* GridSlot) const;
	bool IsInGridBounds(const int32 StartIndex,const FIntPoint& ItemDimension) const;
	bool DoesItemMatchType(const UInventoryItem* SubItem,const FGameplayTag& ItemType) const;
	bool IsUpperLeftIndex(const UInventoryGridSlot* GridSlot,const UInventoryGridSlot* SubGridSlot);
	bool CheckSlotConstraints(
		const UInventoryGridSlot* GridSlot,
		const UInventoryGridSlot* SubGridSlot,
		const TSet<int32>& CheckedSet,
		TSet<int32>& OutIndices,
		const FGameplayTag& ItemType,
		const int32 MaxStackSize);
	bool IsIndexClaimed(const TSet<int32>& CheckedSet,int32 GridIndex)
	{
		return CheckedSet.Contains(GridIndex);
	}
	FIntPoint GetGridDimensions(const FItemManifest& ItemManifest) const;
	bool HasRoomAtIndex(
		const UInventoryGridSlot* GridSlot,
		const FIntPoint GridDimensions,
		const TSet<int32>& CheckedIndices,
		TSet<int32>& OutIndices,
		const FGameplayTag& ItemType,
		const int32 MaxStackSize);
	// item check and set end

	// hover item start
	bool IsRightClick(const FPointerEvent& Event) const;
	bool IsLeftClick(const FPointerEvent& Event) const;
	void PickUp(UInventoryItem* InventoryItem, const int32 GridIndex);
	void AssignHoverItem(UInventoryItem* InventoryItem,const int32 GridIndex,const int32 PreviousIndex);
	void RemoveItemFromGrid(UInventoryItem* InventoryItem,const int32 GridIndex);
	void UpdateTileParameters(const FVector2D& CanvasPosition,const FVector2D& MousePosition);
	FIntPoint CalculateHoveredCoordinates(const FVector2D& CanvasPosition,const FVector2D& MousePosition) const;
	EInventoryTileQuadrant CalculateTileQuadrant(const FVector2D& CanvasPosition,const FVector2D& MousePosition) const;
	void OnTileParameterUpdated(const FInventoryTileParameters& InParameters);
	FIntPoint CalculateStartingCoordinates(const FIntPoint& Coordinate,const FIntPoint& Dimensions,const EInventoryTileQuadrant Quadrant) const;
	FInventorySpaceQueryResult CheckHoverPosition(const FIntPoint& Position,const FIntPoint& Dimensions);
	bool CursorExitedCanvas(const FVector2D& BoundaryPos,const FVector2D& BoundarySize,const FVector2D& Location);
	void HightlightSlots(const int32 Index,const FIntPoint& Dimensions);
	void UnHightlightSlots(const int32 Index,const FIntPoint& Dimensions);
	void ChangeHoverType(const int32 Index,const FIntPoint& Dimensions,EInventoryGridSlotState SlotState);
	void PutItemAtIndex(const int32 Index);
	bool IsSameStackable(const UInventoryItem* ClickedItem) const;
	void SwapWithHoverItem(UInventoryItem* ClickedItem,const int32 GridIndex);
	bool ShouldSwapStackCount(const int32 RoomInClickedSlot,const int32 HoveredStackCount,const int32 MaxStackSize) const;
	void SwapStackCount(const int32 ClickedStackCount,const int32 HoveredStackCount,const int32 Index);
	void ConsumeHoverItemStacks(const int32 ClickedStackCount,const int32 HoveredStackCount,const int32 Index);
	void FillInStack(const int32 FillAmount,const int32 Remainder,const int32 Index);
	void PutHoverItemBack();

	UFUNCTION()
	void OnIventoryMenuToggled(bool bOpen);
	// hover item end
	
	// popup menu start
	void CreatePopupMenu(int32 GridIndex);
	// popup menu end

	// cursor management start
	UUserWidget* GetVisibleCursorWidget();
	UUserWidget* GetHiddenCursorWidget();
public:
	
	void ShowCursor();
	void HideCursor();
	// cursor management end
private:
	TWeakObjectPtr<UInventoryComp> InventoryComp;
	TWeakObjectPtr<UCanvasPanel> OwningCanvasPanel;

	// itemCategory
	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta=(AllowPrivateAccess),Category = "Inventory")
	EInventoryGridType GridType;

	UPROPERTY()
	TArray<TObjectPtr<UInventoryGridSlot>> GridSlots;

	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UInventoryGridSlot> GridSlotClass;

	UPROPERTY(EditAnywhere,Category="Inventory")
	TSubclassOf<UInventorySlottedItem> SlottedItemClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	
	UPROPERTY(EditAnywhere,Category = "Inventory")
	int Row;
	
	UPROPERTY(EditAnywhere,Category = "Inventory")
	int Column;
	
	UPROPERTY(EditAnywhere,Category = "Inventory")
	float TileSize;

	UPROPERTY()
	TMap<int32,TObjectPtr<UInventorySlottedItem>> SlottedItems;

	// hover item start
	UPROPERTY(EditAnywhere,Category = "Inventory")
	TSubclassOf<UInventoryHoverItem> HoverItemClass;

	UPROPERTY()
	TObjectPtr<UInventoryHoverItem> HoverItem;

	FInventoryTileParameters TileParameters;
	FInventoryTileParameters LastTileParameters;

	int32 ItemDropIndex;
	FInventorySpaceQueryResult CurrentQueryResult;

	bool bMouseWithinCanvas;
	bool bLastMouseWithinCanvas;

	FIntPoint LastHightlightDimensions;
	int32 LastHightlightIndex;
	// hover item end

	// cursor start
	UPROPERTY(EditAnywhere,Category = "Inventory")
	TSubclassOf<UUserWidget> VisibleCursorWidgetClass;
	UPROPERTY(EditAnywhere,Category = "Inventory")
	TSubclassOf<UUserWidget> HiddenCursorWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> VisibleCursorWidget;
	UPROPERTY()
	TObjectPtr<UUserWidget> HiddenCursorWidget;
	// cursor end

	// popup menu start
	UPROPERTY(EditAnywhere,Category = "Inventory")
	TSubclassOf<UItemPopUp> PopUpWidgetClass;

	UPROPERTY()
	TObjectPtr<UItemPopUp> PopUpMenuWidget;

	UPROPERTY(EditAnywhere,Category = "Inventory")
	FVector2D ItemPopUpOffset;
	// popup menu end
};
