// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Inventory/Manifest/ItemManifest.h"
#include "InventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryItem : public UObject
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override {return true;}
	
	void SetItemManifest(const FItemManifest& Manifest);

	const FItemManifest& GetItemManifest() const {return ItemManifest.Get<FItemManifest>();}

	// get mutable override
	FItemManifest& GetItemManifest() {return ItemManifest.GetMutable<FItemManifest>();}

	bool IsStackable() const
	{
		const FStackableFragment* Fragment = GetItemManifest().GetFragmentType<FStackableFragment>();
		return Fragment != nullptr;
	};

	bool IsConsumable() const
	{
		return GetItemManifest().GetItemCategory() == EInventoryGridType::Consumable;
	}

	bool IsSellable() const
	{
		const FSellableFragment* Fragment = GetItemManifest().GetFragmentType<FSellableFragment>();
		return Fragment != nullptr;
	}

	int32 GetTotalStackCount() { return TotalStackCount;}

	void SetTotalStackCount(int32 StackCount) { TotalStackCount = StackCount; }
private:
	UPROPERTY(VisibleAnywhere,meta=(BaseStruct="Script/Inventory.FItemManifest"),Replicated)
	FInstancedStruct ItemManifest;

	UPROPERTY(Replicated)
	int32 TotalStackCount{0};
};

template<typename FragmentType>
const FragmentType* GetFragment(const UInventoryItem* Item,const FGameplayTag& FragmentTag)
{
	if (!IsValid(Item)) return nullptr;
	const FItemManifest& Manifest = Item->GetItemManifest();
	return Manifest.GetFragmentTypeWithTag<FragmentType>(FragmentTag);
}
