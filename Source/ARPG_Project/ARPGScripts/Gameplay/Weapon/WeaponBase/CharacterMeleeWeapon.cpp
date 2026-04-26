// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterMeleeWeapon.h"


// Sets default values
ACharacterMeleeWeapon::ACharacterMeleeWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACharacterMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACharacterMeleeWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

