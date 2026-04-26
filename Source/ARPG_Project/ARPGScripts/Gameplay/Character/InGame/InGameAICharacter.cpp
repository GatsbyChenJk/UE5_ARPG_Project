// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameAICharacter.h"

#include "AInGameCharacter.h"
#include "CharacterDamageComponent.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "ARPGScripts/Gameplay/Character/FAIManifest.h"
#include "ARPGScripts/Gameplay/Character/AI/AIWidgetComponent.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/RollComponent.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterAttackComponent.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterShieldComponent.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/ARPGBaseWeapon.h"
#include "HealthManager/CharacterHealthManager.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "StaminaManager/UStaminaManagerComponent.h"


void AInGameAICharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInGameAICharacter, EquipmentActors);
}

AInGameAICharacter::AInGameAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	
	AIAbilitySystemComp = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AIAbilitySystemComp"));
	AIAbilitySystemComp->SetIsReplicated(true);
	AIAbilitySystemComp->SetReplicationMode(EGameplayEffectReplicationMode::Minimal); 
	
	AIAttributeSet = CreateDefaultSubobject<UInGameCharacterAttributeSet>(TEXT("AIAttributeSet"));
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this,&ThisClass::OnPerceptionUpdated);

	AIWidgetComponent = CreateDefaultSubobject<UAIWidgetComponent>(TEXT("AIWidgetComponent"));
	AIWidgetComponent->SetupAttachment(RootComponent);

	//AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = true;
}

void AInGameAICharacter::BeginPlay()
{
	Super::BeginPlay();

	ARPG_EVENT_ADD_UOBJECT(this,FName("AIEquipWeapon"));
	ARPG_EVENT_ADD_UOBJECT(this,FName("AIUnequipWeapon"));
	ARPG_EVENT_ADD_UOBJECT(this,FName("OnHealthChanged"));
	ARPG_EVENT_ADD_UOBJECT(this,FName("OnStaminaChanged"));
}

void AInGameAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInGameAICharacter::AIBasicDataInitialize(FAIManifest AIConfigData)
{
	if (!IsValid(AIAbilitySystemComp) || !HasAuthority()) return;
	
	AIAbilitySystemComp->InitAbilityActorInfo(this, this);

	AICharacterDataInit(AIConfigData);

	AICharacterAbilitiesInit(AIConfigData);

	AIEquipmentInitialize(AIConfigData);
}

void AInGameAICharacter::AICharacterDataInit(FAIManifest AIConfigData)
{
	TSubclassOf<UGameplayEffect> InitEffect = AIConfigData.AIInitData;
    
	//应用GameplayEffect
	FGameplayEffectContextHandle EffectContext = AIAbilitySystemComp->MakeEffectContext();
	EffectContext.AddSourceObject(this);
    
	FGameplayEffectSpecHandle SpecHandle = AIAbilitySystemComp->MakeOutgoingSpec(
		InitEffect, 
		1.0f, 
		EffectContext
	);
    
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveEffectHandle = 
			AIAbilitySystemComp->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            
		UE_LOG(LogTemp, Log, TEXT("AI Applied initial attributes GE. Handle: %s"), 
			   *ActiveEffectHandle.ToString());
	}
}
// Careful!!! Init all AI abilities after this func
void AInGameAICharacter::AICharacterAbilitiesInit(FAIManifest AIConfigData)
{
	AIAbilitySystemComp->ClearAllAbilities();
	// TODO : Grant healing ability,sprint ability

	if (const auto HealthMgrComp = GetHealthManagerComponent())
	{
		HealthMgrComp->Server_InitHealthManagerComponent();
	}

	if (const auto StaminaMgrComp = GetStaminaManagerComponent())
	{
		StaminaMgrComp->Server_InitStaminaManagerComponent();
	}
	
	if (const auto AIDamageComp = GetCharacterDamageComponent())
	{
		AIDamageComp->Server_SetAIGetDamage(AIConfigData);
	}

	if (const auto AIRollComp = GetRollComponent())
	{
		AIRollComp->Server_SetCharacterRollAbility(AIConfigData.IDConfigDataAsset->CharacterIDConfigs.RollAbilityID);
	}
	
}

void AInGameAICharacter::ARPG_AIEquipWeapon(UARPGEventData_Weapon* Event)
{
	if (Event->EquipWeaponType == EWeaponType::Sword)
	{
		if (const auto CharAttackComp = GetCharacterAttackComponent())
		{
			CharAttackComp->Server_InitAIEquipmentAbility(Event->WeaponID);
			CharAttackComp->SetOwningWeapon(Event->WeaponActor);
		}
	}
	else
	{
		if (const auto CharDefenseComp = GetCharacterShieldComponent())
		{
			CharDefenseComp->Server_InitAIEquipmentAbility(Event->WeaponID);
			CharDefenseComp->SetOwningWeapon(Event->WeaponActor);
		}
	}
}

void AInGameAICharacter::ARPG_AIUnequipWeapon(UARPGEventData_Weapon* Event)
{
	if (Event->EquipWeaponType == EWeaponType::Sword)
	{
		if (const auto CharAttackComp = GetCharacterAttackComponent())
		{
			CharAttackComp->Server_RemoveAIEquipmentAbility();
			CharAttackComp->SetOwningWeapon(nullptr);
		}
	}
	else
	{
		if (const auto CharDefenseComp = GetCharacterShieldComponent())
		{
			CharDefenseComp->Server_RemoveAIEquipmentAbility();
			CharDefenseComp->SetOwningWeapon(nullptr);
		}
	}
}

void AInGameAICharacter::ARPG_OnHealthChanged(UARPGEventData* Event)
{
	UARPGEventData_OnCharacterAttributeChanged* Data = Cast<UARPGEventData_OnCharacterAttributeChanged>(Event);
	if (IsValid(Data))
	{
		float Precent = Data->InAttributeCount / Data->AttributeBound;
		if (const auto AIBarWidget = GetWidgetComponent())
		{
			AIBarWidget->SetHealthPercent(Precent);
		}
	}
}

void AInGameAICharacter::ARPG_OnStaminaChanged(UARPGEventData* Event)
{
	// TODO: update data in blackboard var
}

void AInGameAICharacter::AIEquipmentInitialize(FAIManifest AIConfigData)
{
	if (AIConfigData.EquipmentActors.Num() > 0)
	{
		for (auto Equipment : AIConfigData.EquipmentActors)
		{
			if (IsValid(Equipment))
			{
				AActor* EquipItem = GetWorld()->SpawnActor<AActor>(Equipment);
				EquipmentActors.Add(EquipItem);
			}
		}

		UARPGEventData_Equipment* EventData = NewObject<UARPGEventData_Equipment>();
		EventData->Equipments = EquipmentActors;
		ARPG_EVENT_UOBJECT(FName("AIAddEquipWeapon"),EventData);
	}
}

void AInGameAICharacter::CharacterDeath()
{
	Super::CharacterDeath();

	// TODO:Destory AI Equipment actor before destory AI actor
	if (const auto AttackComp = GetCharacterAttackComponent())
	{
		if (auto EquippedWeapon = AttackComp->GetCurrentEquippedWeapon())
		{
			EquippedWeapon->Destroy();
		}
	}

	if (const auto DefenseComp = GetCharacterShieldComponent())
	{
		if (auto EquippedWeapon = DefenseComp->GetCurrentEquippedWeapon())
		{
			EquippedWeapon->Destroy();
		}
	}
	
	this->Destroy();
}


void AInGameAICharacter::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (const auto AIController = GetAIController())
	{
		GEngine->AddOnScreenDebugMessage(-1,0.5f, FColor::Red,FString::Printf(TEXT("Current Perceptive Actors Num:%d"),UpdatedActors.Num()));

		bool bHasPlayerInSight = false;

		TArray<AActor*> Players;
		if (IsValid(AIPerceptionComp))
		{
			AIPerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(),Players);
			if (Players.Num() > 0)
			{
				bHasPlayerInSight = true;
			}
		}

		if (bHasPlayerInSight)
		{
			//TODO: if has multiple players,find the nearest
			AIController->SetBlackboardTarget(Players[0]);

			const float TargetDist = FVector::Dist(Players[0]->GetActorLocation(),GetActorLocation());
			AIController->SetBlackboardFloat("TargetDistance",TargetDist);

			AIController->SetAIControlState(EAIControlState::Attack);
		}
		else
		{
			AIController->SetAIControlState(EAIControlState::Patrol);
		}
	}
}


