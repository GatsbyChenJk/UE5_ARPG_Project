// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Component/InventoryComp.h"

#include "AIController.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameWidgetCompoent.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "Items/Components/ItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/InventoryPlayerController.h"
#include "Widgets/Inventory/InventoryBase/InventoryBaseComp.h"
#include "Inventory/InventoryItem.h"


// Sets default values for this component's properties
UInventoryComp::UInventoryComp() : InventoryList(this)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	//bIsInventoryMenuOpen = false;
}

void UInventoryComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass,InventoryList);
}

void UInventoryComp::TryAddItem(UItemComponent* ItemComp)
{
	if (!InventoryMenu)
	{
		if (AInventoryPlayerController* OwnerController = Cast<AInventoryPlayerController>(OwningController))
		{
			if (const auto WidgetComp = OwnerController->GetInGameWidgetComponent())
			{
				InventoryMenu = Cast<UInventoryBaseComp>(WidgetComp->GetInventoryMenu());
			}
		}
	}
	
	FInventorySlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComp);
	UInventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemComp->GetItemManifest().GetGameplayTag());
	Result.Item = FoundItem;
	
	if (Result.TotalRoomToFill == 0)
	{
		OnNoRoomInInventory.Broadcast();
		return;
	}

	// actual implement for add item logic
	if (Result.Item.IsValid() && Result.bStackable)
	{
		// case 1 : current slot is empty and add want to add is stackable
		OnStackChange.Broadcast(Result);
		ServerAddStackToItem(ItemComp,Result.TotalRoomToFill,Result.Remainder);
	}
	else if (Result.TotalRoomToFill > 0)
	{
		// case 2 : current slot has stackable item(s) and not full
		ServerAddNewItem(ItemComp,Result.bStackable ? Result.TotalRoomToFill : 0,Result.Remainder);
	}
}

void UInventoryComp::ServerAIAddNewItem_Implementation(UItemComponent* ItemComponent)
{
	UInventoryItem* NewItem = InventoryList.AddEntry(ItemComponent);
}

void UInventoryComp::BeginPlay()
{
	Super::BeginPlay();

	OwningController = Cast<APlayerController>(GetOwner());

	if (!OwningController.IsValid())
	{
		OwningAIController = Cast<AAIController>(GetOwner());
	}

	ARPG_EVENT_ADD_UOBJECT(this,FName("AIAddEquipWeapon"));
}

void UInventoryComp::ARPG_AIAddEquipWeapon(UARPGEventData* Event)
{
	UARPGEventData_Equipment* EventData = Cast<UARPGEventData_Equipment>(Event);
	if (IsValid(EventData))
	{
		for (auto Equipment : EventData->Equipments)
		{
			UItemComponent* ItemComp = Equipment->FindComponentByClass<UItemComponent>();
			ServerAIAddNewItem(ItemComp);
		}
	}
}

void UInventoryComp::Server_DropItem_Implementation(UInventoryItem* Item, int32 StackCount)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - StackCount;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	SpawnDroppedItem(Item, StackCount);
}

void UInventoryComp::SpawnDroppedItem(UInventoryItem* Item, int32 StackCount)
{
	const APawn* OwningPawn = OwningController->GetPawn();
	FVector RotatedForward = OwningPawn->GetActorForwardVector();
	RotatedForward = RotatedForward.RotateAngleAxis(FMath::FRandRange(DropSpawnAngleMin, DropSpawnAngleMax), FVector::UpVector);
	FVector SpawnLocation = OwningPawn->GetActorLocation() + RotatedForward * FMath::FRandRange(DropSpawnDistanceMin, DropSpawnDistanceMax);
	SpawnLocation.Z -= RelativeSpawnElevation;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FItemManifest& ItemManifest = Item->GetItemManifest();
	if (FStackableFragment* StackableFragment = ItemManifest.GetFragmentTypeMutable<FStackableFragment>())
	{
		StackableFragment->SetStackCount(StackCount);
	}
	ItemManifest.SpawnPickupActor(this, SpawnLocation, SpawnRotation);
}

void UInventoryComp::Server_SellAllItem_Implementation()
{
	// get all item in InventoryList
	// calculate item sell value
	// apply to character attribute
}

void UInventoryComp::Server_SellItem_Implementation(UInventoryItem* Item)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - 1;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	if (FSellableFragment* SellableFragment = Item->GetItemManifest().GetFragmentTypeMutable<FSellableFragment>())
	{
		SellableFragment->OnSell(OwningController.Get());
	}
	
}

void UInventoryComp::Server_EquipSlotClicked_Implementation(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip)
{
	Multicast_EquipSlotClicked(ItemToEquip, ItemToUnequip);
}

void UInventoryComp::Multicast_EquipSlotClicked_Implementation(UInventoryItem* ItemToEquip,
	UInventoryItem* ItemToUnequip)
{
	// Equipment Component will listen to these delegates
	OnItemEquipped.Broadcast(ItemToEquip);
	OnItemUnequipped.Broadcast(ItemToUnequip);
}

void UInventoryComp::Server_ConsumeItem_Implementation(UInventoryItem* Item)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - 1;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item);
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	if (FConsumableFragment* ConsumableFragment = Item->GetItemManifest().GetFragmentTypeMutable<FConsumableFragment>())
	{
		ConsumableFragment->OnConsume(OwningController.Get());
	}
}

void UInventoryComp::Server_AIEquipWeapon_Implementation(FGameplayTag ItemType)
{
	auto ItemToEquip = FindItemByType(ItemType);
	Multicast_AIEquipWeapon(ItemToEquip, nullptr);
}

void UInventoryComp::Multicast_AIEquipWeapon_Implementation(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip)
{
	OnItemEquipped.Broadcast(ItemToEquip);
	OnItemUnequipped.Broadcast(ItemToUnequip);
}

void UInventoryComp::AddRepSubObj(UObject* SubObj)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && IsValid(SubObj))
	{
		AddReplicatedSubObject(SubObj);
	}
}

UInventoryItem* UInventoryComp::FindItemByType(FGameplayTag ItemType)
{
	return InventoryList.FindFirstItemByType(ItemType);
}

void UInventoryComp::ServerAddNewItem_Implementation(UItemComponent* ItemComp, int32 StackCount,int32 Remainder)
{
	UInventoryItem* NewItem = InventoryList.AddEntry(ItemComp);

	if (GetOwner()->GetNetMode() == NM_ListenServer || GetOwner()->GetNetMode() == NM_Standalone)
	{
		OnItemAdded.Broadcast(NewItem);
		// client replication in fastarray func postreplicatedadd (call automatically)
	}
	
	// implement logic : once item is add,destroy its owning actor
	if (Remainder == 0)
	{
		ItemComp->PickUp();
	}
	else if (FStackableFragment* StackableFrag = ItemComp->GetItemManifest().GetFragmentTypeMutable<FStackableFragment>())
	{
		StackableFrag->SetStackCount(Remainder);
	}
}

void UInventoryComp::ServerAddStackToItem_Implementation(UItemComponent* ItemComp, int32 StackCount, int32 Remainder)
{
	const FGameplayTag& ItemType =	IsValid(ItemComp) ? ItemComp->GetItemManifest().GetGameplayTag() : FGameplayTag::EmptyTag;
	UInventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemType);
	if (!IsValid(FoundItem)) return;
	
	FoundItem->SetTotalStackCount(FoundItem->GetTotalStackCount() + StackCount);

	if (Remainder == 0)
	{
		ItemComp->PickUp();
	}
	else if (FStackableFragment* StackableFrag = ItemComp->GetItemManifest().GetFragmentTypeMutable<FStackableFragment>())
	{
		StackableFrag->SetStackCount(Remainder);
	}
	
}



