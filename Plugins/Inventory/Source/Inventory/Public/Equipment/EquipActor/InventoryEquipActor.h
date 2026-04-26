// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/ARPGBaseWeapon.h"
#include "GameFramework/Actor.h"
#include "InventoryEquipActor.generated.h"

UCLASS()
class INVENTORY_API AInventoryEquipActor : public AARPGBaseWeapon
{
	GENERATED_BODY()

public:
	AInventoryEquipActor();
	
	virtual FGameplayTag GetEquipmentType() const override { return EquipmentType; }
	virtual void SetEquipmentType(FGameplayTag Type) override { EquipmentType = Type; }

private:

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType;
};
