#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Items/Fragment/ItemFragment.h"
#include "Types/InventoryGridType.h"
#include "StructUtils/InstancedStruct.h"
#include "ItemManifest.generated.h"

struct FInventoryFragment;
class UInventoryItem;

USTRUCT(BlueprintType)
struct INVENTORY_API FItemManifest
{
	GENERATED_BODY()

	TArray<TInstancedStruct<FInventoryFragment>>& GetFragmentsMutable() { return Fragments; }
	UInventoryItem* Manifest(UObject* NewOuter);
	
	void SpawnPickupActor(const UObject* WorldContextObject, const FVector& SpawnLocation,
	                      const FRotator& SpawnRotation);

	EInventoryGridType GetItemCategory() const {return ItemCategory;}

	FGameplayTag GetGameplayTag() const{return ItemTag;}

	void AssimilateInventoryFragments(UInventoryCompositeBase* Composite) const;
	
	template<typename T> requires std::derived_from<T,FInventoryFragment>
	const T* GetFragmentTypeWithTag(const FGameplayTag& Tag) const;

	template<typename T> requires std::derived_from<T,FInventoryFragment>
	const T* GetFragmentType() const;

	template<typename T> requires std::derived_from<T,FInventoryFragment>
	T* GetFragmentTypeMutable();

	template<typename T> requires std::derived_from<T,FInventoryFragment>
	TArray<const T*> GetAllFragmentsOfType() const;
	
private:
	UPROPERTY(EditAnywhere,Category="Inventory")
	TArray<TInstancedStruct<FInventoryFragment>> Fragments;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	EInventoryGridType ItemCategory{EInventoryGridType::None};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<AActor> PickupActorClass;

	void ClearFragments();
};

template<typename T> requires std::derived_from<T,FInventoryFragment>
const T* FItemManifest::GetFragmentTypeWithTag(const FGameplayTag& FragmentTag) const
{
	for (const TInstancedStruct<FInventoryFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			if (!FragmentPtr->GetFragmentTag().MatchesTagExact(FragmentTag)) continue;
			return FragmentPtr;
		}
	}
	
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInventoryFragment>
const T* FItemManifest::GetFragmentType() const
{
	for (const TInstancedStruct<FInventoryFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			return FragmentPtr;
		}
	}
	
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInventoryFragment>
T* FItemManifest::GetFragmentTypeMutable()
{
	for (TInstancedStruct<FInventoryFragment>& Fragment : Fragments)
	{
		if (T* FragmentPtr = Fragment.GetMutablePtr<T>())
		{
			return FragmentPtr;
		}
	}
	
	return nullptr;
}

template <typename T> requires std::derived_from<T, FInventoryFragment>
TArray<const T*> FItemManifest::GetAllFragmentsOfType() const
{
	TArray<const T*> Result;
	for (const TInstancedStruct<FInventoryFragment>& Fragment : Fragments)
	{
		if (const T* FragmentPtr = Fragment.GetPtr<T>())
		{
			Result.Add(FragmentPtr);
		}
	}
	return Result;
}
