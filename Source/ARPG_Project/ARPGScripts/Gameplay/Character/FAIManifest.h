#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Combat/InGamePlayerConfig.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/CharacterWeaponConfig.h"
#include "Engine/DataTable.h"
#include "FAIManifest.generated.h"

class AInGameAIController;
class UGameplayEffect;
class UGameplayAbility;
class UBehaviorTree;
class UBlackboardData;

UENUM(BlueprintType,Blueprintable)
enum class EAIControlState : uint8
{
	Patrol UMETA(DisplayName = "Patrol"),
	Attack UMETA(DisplayName = "Attack"),
	Roll   UMETA(DisplayName = "Roll"),
	Defense UMETA(DisplayName = "Defense"),
	Dodge   UMETA(DisplayName = "Dodge"),
	Heal   UMETA(DisplayName = "Heal"),
};

USTRUCT(BlueprintType)
struct FAIBlackboardKeys
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetActor = FName("TargetActor");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName bCanAttack = FName("bCanAttack");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName bLowHealth = FName("bLowHealth");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentPatrolPoint = FName("CurrentPatrolPoint");
};

USTRUCT(BlueprintType)
struct FAIManifest : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AIName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APawn> AIPawnClass;

	// AI GAS start
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AIHurtAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AIHealingAbility;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> AIInitData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInGamePlayerConfig* IDConfigDataAsset;
	// AI GAS end

	// AI control start
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AInGameAIController> AIControllerClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBehaviorTree* AITree;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBlackboardData* AIBlackboard;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAIBlackboardKeys BlackboardKeys;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> PatrollingPath;
	// AI control end

	// AI Equipment start
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AActor>> EquipmentActors;
	// AI Equipment end
};