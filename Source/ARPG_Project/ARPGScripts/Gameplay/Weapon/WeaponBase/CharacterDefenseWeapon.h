// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGBaseWeapon.h"
#include "CharacterDefenseWeapon.generated.h"

UCLASS()
class ARPG_PROJECT_API ACharacterDefenseWeapon : public AARPGBaseWeapon
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACharacterDefenseWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
