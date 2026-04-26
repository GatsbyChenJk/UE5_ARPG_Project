#include "Inventory/Manifest/ItemManifest.h"

#include "Inventory/InventoryItem.h"
#include "Items/Components/ItemComponent.h"
#include "Widgets/Composite/InventoryCompositeBase.h"

UInventoryItem* FItemManifest::Manifest(UObject* NewOuter)
{
	UInventoryItem* Item = NewObject<UInventoryItem>(NewOuter,UInventoryItem::StaticClass());

	Item->SetItemManifest(*this);
	for (auto& Fragment : Item->GetItemManifest().GetFragmentsMutable())
	{
		Fragment.GetMutable().Manifest();
	}
	ClearFragments();
	return Item;
}

void FItemManifest::SpawnPickupActor(const UObject* WorldContextObject, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!IsValid(PickupActorClass) || !IsValid(WorldContextObject)) return;

	AActor* SpawnedActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickupActorClass, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedActor)) return;

	// Set the item manifest, item category, item type, etc.
	UItemComponent* ItemComp = SpawnedActor->FindComponentByClass<UItemComponent>();
	check(ItemComp);

	ItemComp->InitItemManifest(*this);
}

void FItemManifest::AssimilateInventoryFragments(UInventoryCompositeBase* Composite) const
{
	const auto& InventoryItemFragments = GetAllFragmentsOfType<FInventoryItemFragment>();
	for (const auto* Fragment : InventoryItemFragments)
	{
		Composite->ApplyFunction([Fragment](UInventoryCompositeBase* Widget)
		{
			Fragment->Assimilate(Widget);
		});
	}
}

void FItemManifest::ClearFragments()
{
	for (auto& Fragment : Fragments)
	{
		Fragment.Reset();
	}
	Fragments.Empty();
}
