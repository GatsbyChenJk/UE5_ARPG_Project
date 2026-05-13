// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ARPGScripts/Gameplay/Base/ARPGObjectPoolSystem/PoolableActor.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/ARPGBaseWeapon.h"
#include "GameFramework/Actor.h"
#include "InventoryEquipActor.generated.h"

UCLASS()
class INVENTORY_API AInventoryEquipActor : public AARPGBaseWeapon,public IPoolableActor
{
	GENERATED_BODY()

public:
	AInventoryEquipActor();
	
	virtual FGameplayTag GetEquipmentType() const override { return EquipmentType; }
	virtual void SetEquipmentType(FGameplayTag Type) override { EquipmentType = Type; }

	virtual void OnActivateFromPool_Implementation(const FTransform& Transform) override;
	virtual void OnReturnToPool_Implementation() override;
	virtual bool IsActiveInPool_Implementation() const override;

private:

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType;

	bool bIsPooledActive = false;
};
