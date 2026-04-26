// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryHoverItem.generated.h"
struct FGameplayTag;
class UInventoryItem;
class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryHoverItem : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetImageBrush(const FSlateBrush& Brush);
	void UpdateStackCount(const int32 count);
	FGameplayTag GetItemTag() const;
	int32 GetStackCount() const { return StackCount; };
	bool GetIsStackable() const { return bIsStackable; };
	void SetIsStackable(const bool bStackable);
	int32 GetPreviousGridIndex() const { return PreviousGridIndex; };
	void SetPreviousGridIndex(const int32 Index) { PreviousGridIndex = Index; };
	FIntPoint GetGridDimensions() const { return GridDimensions; };
	void SetGridDimensions(const FIntPoint& Dimension) { GridDimensions = Dimension; };
	UInventoryItem* GetInventoryItem() const;
	void SetInventoryItem(UInventoryItem* Item);
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_StackCount;

	int32 PreviousGridIndex;
	FIntPoint GridDimensions;
	TWeakObjectPtr<UInventoryItem> InventoryItem;
	bool bIsStackable{false};
	int32 StackCount{0};
	
};
