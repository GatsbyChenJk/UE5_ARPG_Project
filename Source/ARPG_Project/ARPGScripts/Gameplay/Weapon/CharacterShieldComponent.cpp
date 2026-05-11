// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterShieldComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Character/EAbilityInputID.h"
#include "WeaponBase/ARPGBaseWeapon.h"
#include "WeaponBase/CharacterDefenseWeapon.h"


UCharacterShieldComponent::UCharacterShieldComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UCharacterShieldComponent::FindAndAddEquipmentAbilities(const FString& WeaponID, UAbilitySystemComponent* ASC)
{
	Super::FindAndAddEquipmentAbilities(WeaponID, ASC);

	// add shield ability
	
	FGameplayAbilitySpec AttackSpec(
			WeaponConfig.EquipmentBaseAbilityClass,
			1,                                  
			EAbilityInputBinds::None, 
			this                                
		);

	FGameplayAbilitySpecHandle EquipmentHandle = ASC->GiveAbility(AttackSpec);
	EquipmentSpecHandles.Add(EquipmentHandle);

	ShieldMontage = WeaponConfig.EquipmentMontage;

	// add special ability
	for (auto SpecialAbilityConfig : WeaponConfig.SpecialAbilityConfig)
	{
		FGameplayAbilitySpec SpecialSpec(
			SpecialAbilityConfig.EquipmentAbilityClass,1,
			EAbilityInputBinds::None, this
			);

		FGameplayAbilitySpecHandle SpecialAbilityHandle = ASC->GiveAbility(SpecialSpec);
		SpecialAttackAbilitySpecMap.Add(SpecialAbilityConfig.ActivateType,SpecialAbilityHandle);
	}
	
}

bool UCharacterShieldComponent::StartedOperation_Implementation()
{
	Server_ActivateShieldAbility();

	return true;
}

void UCharacterShieldComponent::CompleteDefense()
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(),ShieldAbilityBlockTag,FGameplayEventData());
}

void UCharacterShieldComponent::CompletedOperation_Implementation(bool bIsInterrupted)
{
	if (!IsValid(GetOwner())) return;
	
	CompleteDefense();
}


AARPGBaseWeapon* UCharacterShieldComponent::GetCurrentEquippedWeapon()
{
	if (IsValid(CurrentWeapon))
	{
		return Cast<ACharacterDefenseWeapon>(CurrentWeapon);
	}
	else
	{
		TArray<AActor*> AttachedWeapons;
		GetOwner()->GetAttachedActors(AttachedWeapons);
		ACharacterDefenseWeapon* WeaponActor = nullptr;
	
		for (AActor* AttachedWeapon : AttachedWeapons)
		{
			WeaponActor = Cast<ACharacterDefenseWeapon>(AttachedWeapon);
			if (WeaponActor != nullptr)
			{
				break;
			}
		}
		return WeaponActor;
	}
}

void UCharacterShieldComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UCharacterShieldComponent::Server_ActivateShieldAbility_Implementation()
{
	FGameplayAbilitySpecHandle ShieldHandle = EquipmentSpecHandles.Num() > 0 ? EquipmentSpecHandles[0] : FGameplayAbilitySpecHandle();
	if (ShieldHandle.IsValid())
	{
		if (AARPGBaseCharacter* OwnerChar = Cast<AARPGBaseCharacter>(GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = OwnerChar->GetAbilitySystemComponent())
			{
				ASC->TryActivateAbility(ShieldHandle);
			}
		}
	}
}

