// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryLeaf.h"
#include "InventoryLeafText.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryLeafText : public UInventoryLeaf
{
	GENERATED_BODY()
public:
	void SetText(const FText& Text) const;
	virtual void NativePreConstruct() override;

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LeafText;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 FontSize{12};
};
