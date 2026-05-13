// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/EquipActor/InventoryEquipActor.h"


AInventoryEquipActor::AInventoryEquipActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
}

void AInventoryEquipActor::OnActivateFromPool_Implementation(const FTransform& Transform)
{
	bIsPooledActive = true;
}

void AInventoryEquipActor::OnReturnToPool_Implementation()
{
	bIsPooledActive = false;
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

bool AInventoryEquipActor::IsActiveInPool_Implementation() const
{
	return bIsPooledActive;
}


