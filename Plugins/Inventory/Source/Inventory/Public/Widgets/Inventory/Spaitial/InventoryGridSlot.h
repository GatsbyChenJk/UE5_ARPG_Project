// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryGridSlot.generated.h"

class UItemPopUp;
class UInventoryItem;
/**
 * 
 */
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridSlotEvent,int32,GridIndex,const FPointerEvent&,MouseEvent);

UENUM(BlueprintType)
enum class EInventoryGridSlotState : uint8
{
	Unoccupied,
	Occupied,
	Selected,
	GrayedOut
};

UCLASS()
class INVENTORY_API UInventoryGridSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	// hover grid start
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	FGridSlotEvent GridSlotClicked;
	FGridSlotEvent GridSlotHovered;
	FGridSlotEvent GridSlotUnhovered;
	// hover grid end

	// basic getter and setter start
	void SetTileIndex(int32 index) {TileIndex = index;};
	int32 GetTileIndex() const {return TileIndex;};
	void SetStackCount(int32 count) {StackCount = count;};
	int32 GetStackCount() const {return StackCount;};
	void SetUpperLeftIndex(int32 Index) {UpperLeftIndex = Index;};
	int32 GetUpperLeftIndex() const {return UpperLeftIndex;};
	UInventoryItem* GetInventoryItem() const {return InventoryItem.Get();};
	void SetInventoryItem(UInventoryItem* Item);
	bool GetAvailable() const {return bAvailable;};
	void SetAvailable(bool bIsAvailable) {bAvailable = bIsAvailable;};
	void SetItemPopup(UItemPopUp* InPopup);
	UItemPopUp* GetItemPopup() const {return ItemPopUp.Get();};

	void SetOccupiedTexture();
	void SetUnoccupiedTexture();
	void SetSelectedTexture();
	void SetGrayedOutTexture();
	// basic setter and getter end
	
private:
	int32 TileIndex{INDEX_NONE};
	int32 StackCount{0};
	int32 UpperLeftIndex{INDEX_NONE};
	TWeakObjectPtr<UInventoryItem> InventoryItem;
	TWeakObjectPtr<UItemPopUp> ItemPopUp;
	bool bAvailable{true};
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_GridSlot;

	UPROPERTY(EditAnywhere,Category="Inventory")
	FSlateBrush Brush_Occupied;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_UnOccupied;

	UPROPERTY(EditAnywhere,Category="Inventory")
	FSlateBrush Brush_Selected;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FSlateBrush Brush_GrayOut;

	EInventoryGridSlotState GridSlotState;

	UFUNCTION()
	void OnItemPopupDestruct(UUserWidget* Menu);
};
