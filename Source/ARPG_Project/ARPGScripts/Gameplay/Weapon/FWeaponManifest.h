#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FWeaponManifest.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum EWeaponType : uint8
{
	Unarmeds	UMETA(DisplayName = "Unarmeds"),
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> EquipmentBaseAbilityClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEquipmentSpecialAbilityConfig> SpecialAbilityConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> DetectSocketNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> WeaponClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboWindow;
};
