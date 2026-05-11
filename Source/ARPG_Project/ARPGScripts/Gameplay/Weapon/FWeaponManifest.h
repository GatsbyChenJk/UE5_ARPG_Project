#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FWeaponManifest.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum EWeaponType : uint8
{
	Unarmed	UMETA(DisplayName = "Unarmed"),
	Sword		UMETA(DisplayName = "Sword"),
	Shield		UMETA(DisplayName = "Shield"),
};

UENUM(BlueprintType)
enum EEquipmentActivateType
{
	LightAttack		UMETA(DisplayName = "轻攻击"),
	HeavyAttack		UMETA(DisplayName = "重攻击"),
	JumpAttack		UMETA(DisplayName = "跳跃攻击"),
	NormalDefense	UMETA(DisplayName = "格挡"),
	BounceBack		UMETA(DisplayName = "弹反")
};

// config for heavy attack, exclusive ability etc..
USTRUCT(blueprintType)
struct FEquipmentSpecialAbilityConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TEnumAsByte<EEquipmentActivateType> ActivateType;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> EquipmentAbilityClass;
};

USTRUCT(BlueprintType)
struct FWeaponManifest : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FString WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EWeaponType> WeaponType;

	// for base equipment ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="WeaponManifest|EquipmentBase")
	TSubclassOf<UGameplayAbility> EquipmentBaseAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="WeaponManifest|EquipmentBase")
	TObjectPtr<UAnimMontage> EquipmentMontage;
	// ------------------------------------------

	// specially for combo ability weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="WeaponManifest|Combo")
	TSubclassOf<UGameplayAbility> ComboAbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="WeaponManifest|Combo")
	TArray<TObjectPtr<UAnimMontage>> ComboMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="WeaponManifest|Combo")
	float ComboWindow;
	// --------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEquipmentSpecialAbilityConfig> SpecialAbilityConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> DetectSocketNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> WeaponClass;
};
