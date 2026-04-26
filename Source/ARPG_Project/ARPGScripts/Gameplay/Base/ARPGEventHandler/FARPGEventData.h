#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Weapon/FWeaponManifest.h"
#include "UObject/Object.h"

#include "FARPGEventData.generated.h"

class AARPGBaseWeapon;

UCLASS()
class UARPGEventData : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	AActor* Instigator = nullptr;

	// 死亡目标（当前Actor）
	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	AActor* TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	FString InfoData = TEXT("Data");
};

UCLASS()
class UARPGEventData_OnCharacterAttributeChanged : public UARPGEventData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	float InAttributeCount = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	float AttributeBound = 0.0f;
};

UCLASS()
class UARPGEventData_Weapon : public UARPGEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	TEnumAsByte<EWeaponType> EquipWeaponType;

	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	FString WeaponID = TEXT("W00");

	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	AARPGBaseWeapon* WeaponActor = nullptr;
};

UCLASS()
class UARPGEventData_Equipment : public UARPGEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "ARPGEvent")
	TArray<AActor*> Equipments;
};