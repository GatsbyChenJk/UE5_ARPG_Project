// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Component/InventoryEquipmentComponent.h"

#include "ARPGScripts/Gameplay/AIController/InGameAIController.h"
#include "Equipment/EquipActor/InventoryEquipActor.h"
#include "GameFramework/Character.h"
#include "Inventory/InventoryItem.h"
#include "Inventory/Component/InventoryComp.h"
#include "Inventory/Manifest/ItemManifest.h"
#include "Inventory/Util/InventoryStatics.h"
#include "Items/Fragment/ItemFragment.h"

UInventoryEquipmentComponent::UInventoryEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryEquipmentComponent::SetOwningSkeletalMesh(USkeletalMeshComponent* OwningMesh)
{
	OwningSkeletalMesh = OwningMesh;
}

void UInventoryEquipmentComponent::InitializeOwner(APlayerController* PlayerController)
{
	if (IsValid(PlayerController))
	{
		OwningPlayerController = PlayerController;
	}
	InitInventoryComponent();
}

void UInventoryEquipmentComponent::OnItemEquipped(UInventoryItem* EquippedItem)
{
	if (!IsValid(EquippedItem)) return;
	if (!bIsProxy && !OwningPlayerController->HasAuthority()) return;

	FItemManifest& ItemManifest = EquippedItem->GetItemManifest();
	FEquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentTypeMutable<FEquipmentFragment>();
	if (!EquipmentFragment) return;

	if (!OwningSkeletalMesh.IsValid()) return;
	AInventoryEquipActor* SpawnedEquipActor = SpawnEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());

	if (!bIsProxy)
	{
		EquipmentFragment->OnEquip(OwningPlayerController.Get());
	}

	// if (!OwningSkeletalMesh.IsValid()) return;
	// AInventoryEquipActor* SpawnedEquipActor = SpawnEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());

	EquippedActors.Add(SpawnedEquipActor);
}

void UInventoryEquipmentComponent::OnItemUnequipped(UInventoryItem* UnequippedItem)
{
	if (!IsValid(UnequippedItem)) return;
	if (!bIsProxy && !OwningPlayerController->HasAuthority()) return;

	FItemManifest& ItemManifest = UnequippedItem->GetItemManifest();
	FEquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentTypeMutable<FEquipmentFragment>();
	if (!EquipmentFragment) return;

	if (!bIsProxy)
	{
		EquipmentFragment->OnUnequip(OwningPlayerController.Get());
	}

	RemoveEquippedActor(EquipmentFragment->GetEquipmentType());
}

void UInventoryEquipmentComponent::OnAIItemEquipped(UInventoryItem* EquippedItem)
{
	if (!IsValid(EquippedItem)) return;
	if (!OwningAIController->HasAuthority()) return;

	FItemManifest& ItemManifest = EquippedItem->GetItemManifest();
	FEquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentTypeMutable<FEquipmentFragment>();
	if (!EquipmentFragment) return;

	if (!OwningSkeletalMesh.IsValid()) return;
	AInventoryEquipActor* SpawnedEquipActor = SpawnEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());

	if (!bIsProxy)
	{
		EquipmentFragment->OnAIEquip(OwningAIController.Get());
	}

	// if (!OwningSkeletalMesh.IsValid()) return;
	// AInventoryEquipActor* SpawnedEquipActor = SpawnEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());

	EquippedActors.Add(SpawnedEquipActor);
}

void UInventoryEquipmentComponent::OnAIItemUnequipped(UInventoryItem* UnequippedItem)
{
	if (!IsValid(UnequippedItem)) return;
	if (!OwningAIController->HasAuthority()) return;

	FItemManifest& ItemManifest = UnequippedItem->GetItemManifest();
	FEquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentTypeMutable<FEquipmentFragment>();
	if (!EquipmentFragment) return;

	if (!bIsProxy)
	{
		EquipmentFragment->OnAIUnequip(OwningAIController.Get());
	}

	RemoveEquippedActor(EquipmentFragment->GetEquipmentType());
}

void UInventoryEquipmentComponent::InitPlayerController()
{
	if (OwningPlayerController = Cast<APlayerController>(GetOwner()); OwningPlayerController.IsValid())
	{
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn()); IsValid(OwnerCharacter))
		{
			OnPossessedPawnChange(nullptr, OwnerCharacter);
		}
		else
		{
			OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChange);
		}
	}
	else
	{
		InitAIController();
	}
}

void UInventoryEquipmentComponent::InitAIController()
{
	if (OwningAIController = Cast<AAIController>(GetOwner()); OwningAIController.IsValid())
	{
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwningAIController->GetPawn()); IsValid(OwnerCharacter))
		{
			OnAIPossessedPawnChange(nullptr, OwnerCharacter);
		}
		else
		{
			OwningAIController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnAIPossessedPawnChange);
		}
	}
}

void UInventoryEquipmentComponent::InitInventoryComponent()
{
	InventoryComponent = UInventoryStatics::GetInventoryComp(OwningPlayerController.Get());
	
	if (!InventoryComponent.IsValid()) return;

	if (!InventoryComponent->OnItemEquipped.IsAlreadyBound(this, &ThisClass::OnItemEquipped))
	{
		InventoryComponent->OnItemEquipped.AddDynamic(this, &ThisClass::OnItemEquipped);
	}

	if (!InventoryComponent->OnItemUnequipped.IsAlreadyBound(this, &ThisClass::OnItemUnequipped))
	{
		InventoryComponent->OnItemUnequipped.AddDynamic(this, &ThisClass::OnItemUnequipped);
	}
}

void UInventoryEquipmentComponent::InitAIInventoryComponent()
{
	InventoryComponent = UInventoryStatics::GetAIInventoryComp(OwningAIController.Get());
	
	if (!InventoryComponent.IsValid()) return;

	if (!InventoryComponent->OnItemEquipped.IsAlreadyBound(this, &ThisClass::OnAIItemEquipped))
	{
		InventoryComponent->OnItemEquipped.AddDynamic(this, &ThisClass::OnAIItemEquipped);
	}

	if (!InventoryComponent->OnItemUnequipped.IsAlreadyBound(this, &ThisClass::OnAIItemUnequipped))
	{
		InventoryComponent->OnItemUnequipped.AddDynamic(this, &ThisClass::OnAIItemUnequipped);
	}
}

AInventoryEquipActor* UInventoryEquipmentComponent::SpawnEquippedActor(FEquipmentFragment* EquipmentFragment,
                                                                       const FItemManifest& Manifest, USkeletalMeshComponent* AttachMesh)
{
	AInventoryEquipActor* SpawnedEquipActor = EquipmentFragment->SpawnAttachedActor(AttachMesh);
	SpawnedEquipActor->SetEquipmentType(EquipmentFragment->GetEquipmentType());
	SpawnedEquipActor->SetOwner(GetOwner());
	if (!bIsProxy)
	{
		SpawnedEquipActor->SetReplicates(true);
		EquipmentFragment->SetEquippedActor(SpawnedEquipActor);
	}
	return SpawnedEquipActor;
}

AInventoryEquipActor* UInventoryEquipmentComponent::FindEquippedActor(const FGameplayTag& EquipmentTypeTag)
{
	auto FoundActor = EquippedActors.FindByPredicate([&EquipmentTypeTag](const AInventoryEquipActor* EquippedActor)
	{
		return EquippedActor->GetEquipmentType().MatchesTagExact(EquipmentTypeTag);
	});
	return FoundActor ? *FoundActor : nullptr;
}

void UInventoryEquipmentComponent::RemoveEquippedActor(const FGameplayTag& EquipmentTypeTag)
{
	if (AInventoryEquipActor* EquippedActor = FindEquippedActor(EquipmentTypeTag); IsValid(EquippedActor))
	{
		EquippedActors.Remove(EquippedActor);
		EquippedActor->Destroy();
	}
}

void UInventoryEquipmentComponent::OnPossessedPawnChange(APawn* OldPawn, APawn* NewPawn)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(NewPawn); IsValid(OwnerCharacter))
	{
		OwningSkeletalMesh = OwnerCharacter->GetMesh();
	}
	InitInventoryComponent();
}

void UInventoryEquipmentComponent::OnAIPossessedPawnChange(APawn* OldPawn, APawn* NewPawn)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(NewPawn); IsValid(OwnerCharacter))
	{
		OwningSkeletalMesh = OwnerCharacter->GetMesh();
	}
	InitAIInventoryComponent();
}


void UInventoryEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitPlayerController();
}

