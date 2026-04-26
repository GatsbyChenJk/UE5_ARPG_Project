// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItem.h"

#include "Net/UnrealNetwork.h"


void UInventoryItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass,ItemManifest)
	DOREPLIFETIME(ThisClass,TotalStackCount)
}

void UInventoryItem::SetItemManifest(const FItemManifest& Manifest)
{
	ItemManifest = FInstancedStruct::Make<FItemManifest>(Manifest);
}
