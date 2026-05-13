// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Components/ItemComponent.h"

#include "ARPGScripts/Gameplay/Base/ARPGObjectPoolSystem/PoolSubsystem.h"
#include "ARPGScripts/Gameplay/Character/Item/PickableItem.h"
#include "Net/UnrealNetwork.h"


UItemComponent::UItemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PickUpMessage = FString("Press E to Pick Up");
	SetIsReplicatedByDefault(true);
}

void UItemComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass,ItemManifest);
}

void UItemComponent::PickUp()
{
	OnPickUp();
	if (auto ItemActor = Cast<APickableItem>(GetOwner()))
	{
		if (UPoolSubsystem* PoolSub = GetWorld()->GetGameInstance()->GetSubsystem<UPoolSubsystem>())
		{
			PoolSub->ReleaseActor(this, ItemActor);
		}
		else
		{
			ItemActor->Destroy();
		}
	}
}

void UItemComponent::OnPickUp_Implementation()
{
}

void UItemComponent::InitItemManifest(FItemManifest CopyOfManifest)
{
	ItemManifest = CopyOfManifest;
}

