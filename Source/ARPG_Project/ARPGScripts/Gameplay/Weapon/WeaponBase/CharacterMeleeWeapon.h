// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGBaseWeapon.h"
#include "CharacterMeleeWeapon.generated.h"

UCLASS()
class ARPG_PROJECT_API ACharacterMeleeWeapon : public AARPGBaseWeapon
{
	GENERATED_BODY()

public:
	ACharacterMeleeWeapon();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
