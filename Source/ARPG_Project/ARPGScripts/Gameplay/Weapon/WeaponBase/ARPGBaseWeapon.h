// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ARPGBaseWeapon.generated.h"

class UCharacterWeaponConfig;

UCLASS()
class ARPG_PROJECT_API AARPGBaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	AARPGBaseWeapon();

	UFUNCTION(BlueprintCallable)
	UStaticMeshComponent* GetCachedMeshComponent();

	virtual FGameplayTag GetEquipmentType() const { return FGameplayTag::EmptyTag;}
	virtual void SetEquipmentType(FGameplayTag Type){}

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* CachedWeaponMeshComp;
};
