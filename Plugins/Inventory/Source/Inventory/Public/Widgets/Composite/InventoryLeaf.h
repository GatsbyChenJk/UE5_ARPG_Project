// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryCompositeBase.h"
#include "InventoryLeaf.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryLeaf : public UInventoryCompositeBase
{
	GENERATED_BODY()
public:
	virtual void ApplyFunction(FuncType Function) override;
};
