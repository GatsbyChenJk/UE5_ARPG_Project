// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableItem.h"


APickableItem::APickableItem()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetReplicates(true);
}

void APickableItem::BeginPlay()
{
	Super::BeginPlay();
}

void APickableItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickableItem::OnActivateFromPool_Implementation(const FTransform& Transform)
{
	bIsPooledActive = true;
}

void APickableItem::OnReturnToPool_Implementation()
{
	bIsPooledActive = false;
}

bool APickableItem::IsActiveInPool_Implementation() const
{
	return bIsPooledActive;
}

