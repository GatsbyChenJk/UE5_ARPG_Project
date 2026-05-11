// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquipmentComponent.h"
#include "GameplayTagContainer.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/SpecialOperation.h"
#include "Components/ActorComponent.h"
#include "CharacterShieldComponent.generated.h"


class ACharacterDefenseWeapon;
struct FGameplayAbilitySpecHandle;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UCharacterShieldComponent : public UEquipmentComponent,public ISpecialOperation
{
	GENERATED_BODY()

public:
	UCharacterShieldComponent();

	virtual void FindAndAddEquipmentAbilities(const FString& WeaponID, UAbilitySystemComponent* ASC) override;
	
	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_ActivateShieldAbility();

	virtual bool StartedOperation_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void CompleteDefense();

	virtual void CompletedOperation_Implementation(bool bIsInterrupted = false) override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Shield Ability")
	FGameplayTag ShieldAbilityBlockTag = FGameplayTag();
	

	virtual AARPGBaseWeapon* GetCurrentEquippedWeapon() override;

	virtual UAnimMontage* GetCurrentMontage() override
	{
		return ShieldMontage;
	};
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UPROPERTY(Replicated)
	TObjectPtr<UAnimMontage> ShieldMontage;
};
