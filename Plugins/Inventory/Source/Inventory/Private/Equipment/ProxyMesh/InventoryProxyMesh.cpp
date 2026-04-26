// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/ProxyMesh/InventoryProxyMesh.h"

#include "Equipment/Component/InventoryEquipmentComponent.h"
#include "GameFramework/Character.h"


AInventoryProxyMesh::AInventoryProxyMesh()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetupAttachment(RootComponent);

	EquipmentComponent = CreateDefaultSubobject<UInventoryEquipmentComponent>("Equipment");
	EquipmentComponent->SetOwningSkeletalMesh(Mesh);
	EquipmentComponent->SetIsProxy(true);
}

void AInventoryProxyMesh::BeginPlay()
{
	Super::BeginPlay();

	DelayedInitializeOwner();
}

void AInventoryProxyMesh::DelayedInitializeOwner()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		DelayedInitialization();
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		DelayedInitialization();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
	if (!IsValid(Character))
	{
		DelayedInitialization();
		return;
	}

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!IsValid(CharacterMesh))
	{
		DelayedInitialization();
		return;
	}

	SourceMesh = CharacterMesh;
	Mesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());
	Mesh->SetAnimInstanceClass(SourceMesh->GetAnimInstance()->GetClass());

	EquipmentComponent->InitializeOwner(PC);
}

void AInventoryProxyMesh::DelayedInitialization()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ThisClass::DelayedInitializeOwner);
	GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
}
