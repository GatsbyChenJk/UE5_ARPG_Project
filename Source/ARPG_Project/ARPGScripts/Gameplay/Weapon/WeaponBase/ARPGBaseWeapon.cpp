// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGBaseWeapon.h"

#include "CharacterWeaponConfig.h"


AARPGBaseWeapon::AARPGBaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	this->bReplicates = true;

	CachedWeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	CachedWeaponMeshComp->SetupAttachment(RootComponent);
}

UStaticMeshComponent* AARPGBaseWeapon::GetCachedMeshComponent()
{
	return CachedWeaponMeshComp;
}

void AARPGBaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

