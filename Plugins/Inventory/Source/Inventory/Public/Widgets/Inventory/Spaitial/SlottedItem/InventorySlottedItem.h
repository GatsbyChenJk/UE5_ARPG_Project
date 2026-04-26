// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlottedItem.generated.h"

class UTextBlock;
class UImage;
class UInventoryItem;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlottedItemClicked,int32,ClickIndex,const FPointerEvent&,MouseEvent);

UCLASS()
class INVENTORY_API UInventorySlottedItem : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	void SetIsStackable(bool bStackable) { bIsStackable = bStackable; }
	UImage* GetImageIcon() const {return Image_Icon;}
	void SetGridIndex(int32 Index) {GridIndex = Index;}
	int32 GetGridIndex() const {return GridIndex;}
	void SetGridDimensions(const FIntPoint& Dimensions) {GridDimensions = Dimensions;}
	FIntPoint GetGridDimensions() const {return GridDimensions;}
	void SetInventoryItem(UInventoryItem* Item);
	UInventoryItem* GetInventoryItem() const;
	void SetImageBrush(const FSlateBrush& Brush) const;
	void UpdateStackCount(int32 StackCount);

	FOnSlottedItemClicked OnSlottedItemClicked;
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;

	int32 GridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInventoryItem> InventoryItem;
	bool bIsStackable{false};
};
