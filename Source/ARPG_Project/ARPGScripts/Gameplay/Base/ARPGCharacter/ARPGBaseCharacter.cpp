// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGBaseCharacter.h"

#include "ARPGScripts/Gameplay/Character/InGame/CharacterDamageComponent.h"
#include "ARPGScripts/Gameplay/Character/InGame/HealthManager/CharacterHealthManager.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/RollComponent.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/SprintComponent.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterAttackComponent.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterShieldComponent.h"
#include "ARPGScripts/Gameplay/Character/InGame/StaminaManager/UStaminaManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "GenericTeamAgentInterface.h"


AARPGBaseCharacter::AARPGBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CachedSprintComp = CreateDefaultSubobject<USprintComponent>(TEXT("SprintComp"));
	CachedAttackComp = CreateDefaultSubobject<UCharacterAttackComponent>(TEXT("AttackComp"));
	CachedDamageComp = CreateDefaultSubobject<UCharacterDamageComponent>(TEXT("DamageComp"));
	CachedRollComp = CreateDefaultSubobject<URollComponent>(TEXT("RollComp"));
	CachedShieldComp = CreateDefaultSubobject<UCharacterShieldComponent>(TEXT("ShieldComp"));
	CachedStaminaMgrComp = CreateDefaultSubobject<UStaminaManagerComponent>(TEXT("StaminaMgrComp"));
	CachedHealthMgrComp = CreateDefaultSubobject<UCharacterHealthManager>(TEXT("HealthMgrComp"));
}

void AARPGBaseCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AARPGBaseCharacter,bIsDying)
}

void AARPGBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AARPGBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UCharacterHealthManager* AARPGBaseCharacter::GetHealthManagerComponent()
{
	return CachedHealthMgrComp;
}

UCharacterDamageComponent* AARPGBaseCharacter::GetCharacterDamageComponent()
{
	if (IsValid(CachedDamageComp))
	{
		return CachedDamageComp;
	}
	else
	{
		CachedDamageComp = FindComponentByClass<UCharacterDamageComponent>();
		return CachedDamageComp;
	}
}

UCharacterAttackComponent* AARPGBaseCharacter::GetCharacterAttackComponent()
{
	if (IsValid(CachedAttackComp))
	{
		return CachedAttackComp;
	}
	else
	{
		CachedAttackComp = FindComponentByClass<UCharacterAttackComponent>();
		return CachedAttackComp;
	}
}

void AARPGBaseCharacter::CharacterDeath()
{
	//GEngine->AddOnScreenDebugMessage(-1,2.0f, FColor::Red, FString::Printf(TEXT("Dead Character : %s"),*this->GetName()));
	//SetIsDead(true);
}

UStaminaManagerComponent* AARPGBaseCharacter::GetStaminaManagerComponent() const
{
	return CachedStaminaMgrComp;
}

USprintComponent* AARPGBaseCharacter::GetSprintComponent() const
{
	if (IsValid(CachedSprintComp))
	{
		return CachedSprintComp;
	}
	else
	{
		return FindComponentByClass<USprintComponent>();
	}
}

void AARPGBaseCharacter::SetSprintConsumeHandle(FActiveGameplayEffectHandle InHandle)
{
}

void AARPGBaseCharacter::SetSprintRecoverHandle(FActiveGameplayEffectHandle InHandle)
{
}

URollComponent* AARPGBaseCharacter::GetRollComponent() const
{
	if (IsValid(CachedRollComp))
	{
		return CachedRollComp;
	}
	else
	{
		return FindComponentByClass<URollComponent>();
	}
}

UCharacterShieldComponent* AARPGBaseCharacter::GetCharacterShieldComponent() const
{
	if (IsValid(CachedShieldComp))
	{
		return CachedShieldComp;
	}
	else
	{
		return FindComponentByClass<UCharacterShieldComponent>();
	}
}

void AARPGBaseCharacter::SetTeamID(int32 InTeamID)
{
	if (HasAuthority())
	{
		DefaultTeamID = InTeamID;

		// 同步到Controller（如果存在）
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->SetGenericTeamId(FGenericTeamId(InTeamID));
		}
	}
}

bool AARPGBaseCharacter::IsHostileTo(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	// 获取对方的阵营
	int32 OtherTeamID = -1;
	if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherActor))
	{
		OtherTeamID = TeamAgent->GetGenericTeamId().GetId();
	}
	else if (const AActor* Other = Cast<AActor>(OtherActor))
	{
		// 尝试从Controller获取
		if (const AAIController* AIController = Cast<AAIController>(Other->GetInstigatorController()))
		{
			OtherTeamID = AIController->GetGenericTeamId().GetId();
		}
		else if (const APlayerController* PC = Cast<APlayerController>(Other->GetInstigatorController()))
		{
			OtherTeamID = 0; // 默认玩家阵营
		}
	}

	// 相同阵营为友方，不同阵营为敌方
	return OtherTeamID != -1 && OtherTeamID != GetTeamID();
}

bool AARPGBaseCharacter::IsFriendlyTo(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return false;
	}

	// 获取对方的阵营
	int32 OtherTeamID = -1;
	if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OtherActor))
	{
		OtherTeamID = TeamAgent->GetGenericTeamId().GetId();
	}
	else if (const AActor* Other = Cast<AActor>(OtherActor))
	{
		// 尝试从Controller获取
		if (const AAIController* AIController = Cast<AAIController>(Other->GetInstigatorController()))
		{
			OtherTeamID = AIController->GetGenericTeamId().GetId();
		}
		else if (const APlayerController* PC = Cast<APlayerController>(Other->GetInstigatorController()))
		{
			OtherTeamID = 0; // 默认玩家阵营
		}
	}

	// 相同阵营为友方
	return OtherTeamID == GetTeamID();
}



