#pragma once


#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryFastArray.generated.h"

struct FGameplayTag;
class UInventoryComp;
class UInventoryItem;
class UItemComponent;

USTRUCT(blueprintType)
struct FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryEntry(){}
private:
	friend struct FInventoryFastArray;
	friend UInventoryComp;

	UPROPERTY()
	TObjectPtr<UInventoryItem> Item = nullptr;
};

USTRUCT(blueprintType)
struct FInventoryFastArray : public FFastArraySerializer
{
	GENERATED_BODY()

	FInventoryFastArray():OwnerComponent(nullptr){}
	FInventoryFastArray(UActorComponent* Component) : OwnerComponent(Component){}

	TArray<UInventoryItem*> GetAllItems() const;
	// for fastarrayserializer
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices,int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices,int32 FinalSize);	
	// end for fastarrayseriallizer

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInventoryEntry,FInventoryFastArray>(Entries,DeltaParams,*this);
	}

	UInventoryItem* AddEntry(UItemComponent* ItemComp);
	UInventoryItem* AddEntry(UInventoryItem* Item);
	void RemoveEntry(UInventoryItem* Item);
	UInventoryItem* FindFirstItemByType(const FGameplayTag& ItemType);
	
	
private:
	friend UInventoryComp;

	UPROPERTY()
	TArray<FInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FInventoryFastArray> : public TStructOpsTypeTraitsBase2<FInventoryFastArray>
{
	enum {WithNetDeltaSerializer = true};
};