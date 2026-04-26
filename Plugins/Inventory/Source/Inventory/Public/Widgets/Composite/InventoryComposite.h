// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryCompositeBase.h"
#include "InventoryComposite.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryComposite : public UInventoryCompositeBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void ApplyFunction(FuncType Function) override;
	virtual void Collapse() override;
	TArray<UInventoryCompositeBase*> GetChildren() { return Children; }
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UInventoryCompositeBase>> Children;
};
