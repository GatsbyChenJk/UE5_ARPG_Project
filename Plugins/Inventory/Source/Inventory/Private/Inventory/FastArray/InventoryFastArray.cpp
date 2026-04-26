#include "Inventory/FastArray/InventoryFastArray.h"

#include "Inventory/InventoryItem.h"
#include "Inventory/Component/InventoryComp.h"
#include "Items/Components/ItemComponent.h"

TArray<UInventoryItem*> FInventoryFastArray::GetAllItems() const
{
	TArray<UInventoryItem*> Result;
	Result.Reserve(Entries.Num());
	for (const auto& Entry : Entries)
	{
		if (!IsValid(Entry.Item)) continue;
		Result.Add(Entry.Item);
	}
	return Result;
}

void FInventoryFastArray::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	UInventoryComp* ItemComp = Cast<UInventoryComp>(OwnerComponent);
	if (!IsValid(ItemComp)) return;

	for (int32 Index : RemovedIndices)
	{
		ItemComp->OnItemRemoved.Broadcast(Entries[Index].Item);
	}
}

void FInventoryFastArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	UInventoryComp* ItemComp = Cast<UInventoryComp>(OwnerComponent);
	if (!IsValid(ItemComp)) return;

	for (int32 Index : AddedIndices)
	{
		ItemComp->OnItemAdded.Broadcast(Entries[Index].Item);
	}
}

UInventoryItem* FInventoryFastArray::AddEntry(UItemComponent* ItemComp)
{
	check(OwnerComponent);
	AActor* OwnerActor = OwnerComponent->GetOwner();
	check(OwnerActor->HasAuthority());
	UInventoryComp* IC = Cast<UInventoryComp>(OwnerComponent);
	if (!IsValid(IC)) return nullptr;

	FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = ItemComp->GetItemManifest().Manifest(OwnerActor);

	IC->AddRepSubObj(NewEntry.Item);
	MarkItemDirty(NewEntry);

	return NewEntry.Item;
}

UInventoryItem* FInventoryFastArray::AddEntry(UInventoryItem* Item)
{
	check(OwnerComponent);
	AActor* Owner = OwnerComponent->GetOwner();
	check(Owner->HasAuthority());

	FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = Item;

	MarkItemDirty(NewEntry);
	return Item;
	
}

void FInventoryFastArray::RemoveEntry(UInventoryItem* Item)
{
	for (auto It = Entries.CreateIterator(); It; ++It)
	{
		FInventoryEntry& Entry = *It;
		if (Entry.Item == Item)
		{
			It.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

UInventoryItem* FInventoryFastArray::FindFirstItemByType(const FGameplayTag& ItemType)
{
	auto* FoundItem = Entries.FindByPredicate([Type = ItemType](const FInventoryEntry& Entry)
	{
		return IsValid(Entry.Item) && Entry.Item->GetItemManifest().GetGameplayTag().MatchesTagExact(Type);
	});

	return FoundItem ? FoundItem->Item : nullptr;
}
