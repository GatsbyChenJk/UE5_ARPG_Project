#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "FWeaponManifest.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

// Handle GAS logic for weapon ability
// weapon actor equip logic impl in InventoryEquipmentComponent

class AARPGBaseCharacter;
class AARPGBaseWeapon;
struct FWeaponManifest;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UEquipmentComponent();

	UFUNCTION(Server,Reliable)
	void Server_InitAIEquipmentAbility(const FString& WeaponID);

	UFUNCTION(Server,Reliable)
	void Server_InitCharacterEquipmentAbility(const FString& WeaponID);

	UFUNCTION(Server,Reliable)
	void Server_RemoveAIEquipmentAbility();

	UFUNCTION(Server,Reliable)
	void Server_RemoveCharacterEquipmentAbility();

	UFUNCTION(BlueprintCallable)
	virtual AARPGBaseWeapon* GetCurrentEquippedWeapon();

	UFUNCTION()
	virtual void SetOwningWeapon(AARPGBaseWeapon* InWeapon);

	UFUNCTION(BlueprintCallable)
	float GetEquipmentAttackAttribute(AARPGBaseCharacter* OwnerChar);

	UFUNCTION(BlueprintCallable)
	float GetEquipmentStaminaAttribute(AARPGBaseCharacter* OwnerChar);

	UFUNCTION(BlueprintCallable)
	float GetEquipmentDefenseAttribute(AARPGBaseCharacter* OwnerChar);

	UFUNCTION(BlueprintCallable)
	float GetEquipmentDefenseStaminaAttribute(AARPGBaseCharacter* OwnerChar);
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void RemoveEquipmentAbility(UAbilitySystemComponent* ASC);

	virtual void FindAndAddEquipmentAbilities(const FString& WeaponID, UAbilitySystemComponent* ASC);

	virtual void ActivateSpecialAbilities(UAbilitySystemComponent* ASC, EEquipmentActivateType ActivateType) {};
	
	UPROPERTY(Replicated)
	AARPGBaseWeapon* CurrentWeapon;
	
	TArray<FGameplayAbilitySpecHandle> EquipmentSpecHandles;
	
	TMap<TEnumAsByte<EEquipmentActivateType>,FGameplayAbilitySpecHandle> SpecialAttackAbilitySpecMap;

	FWeaponManifest WeaponConfig;

	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;
};
