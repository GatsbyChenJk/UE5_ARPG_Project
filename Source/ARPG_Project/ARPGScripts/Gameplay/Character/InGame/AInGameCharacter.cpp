// Fill out your copyright notice in the Description page of Project Settings.


#include "AInGameCharacter.h"

#include "CharacterDamageComponent.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagsManager.h"
#include "ARPGScripts/Gameplay/Base/GameModes/InGameMode.h"
#include "ARPGScripts/Gameplay/Base/GameStates/CharacterManager.h"
#include "ARPGScripts/Gameplay/Combat/AInGamePlayerState.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterAttackComponent.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/SprintComponent.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/RollComponent.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterShieldComponent.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/ARPGBaseWeapon.h"


AAInGameCharacter::AAInGameCharacter()
{
	bReplicates = true;
	CachedPlayerTargetLockComp = CreateDefaultSubobject<UPlayerTargetLockComp>(TEXT("TargetLockComp"));
}

void AAInGameCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}

void AAInGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = Cast<UGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (UCharacterManager* CharMgr = GameInstance->GetSubsystem<UCharacterManager>())
		{
			CharMgr->RegisterCharacter(this);
		}
	}

	ARPG_EVENT_ADD_UOBJECT(this,FName("CharacterEquipWeapon"));
	ARPG_EVENT_ADD_UOBJECT(this,FName("CharacterUnequipWeapon"));
}

void AAInGameCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UGameInstance* GameInstance = Cast<UGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (UCharacterManager* CharMgr = GameInstance->GetSubsystem<UCharacterManager>())
		{
			CharMgr->UnRegisterCharacter(this);
		}
	}
}

void AAInGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// attack
		EnhancedInputComponent->BindAction(AttackAction,ETriggerEvent::Started, this, &ThisClass::OnCharacterAttack);
		// sprint
		EnhancedInputComponent->BindAction(RunningAction,ETriggerEvent::Started,this,&ThisClass::OnStartRunning);
		EnhancedInputComponent->BindAction(RunningAction,ETriggerEvent::Completed,this,&ThisClass::OnStopRunning);
		// defense
		EnhancedInputComponent->BindAction(DenfenseAction,ETriggerEvent::Started,this,&ThisClass::OnCharacterDefenseStart);
		EnhancedInputComponent->BindAction(DenfenseAction,ETriggerEvent::Completed,this,&ThisClass::OnCharacterDefenseEnd);
		// roll
		EnhancedInputComponent->BindAction(RollAction,ETriggerEvent::Triggered,this,&ThisClass::OnRolling);

		// test
		EnhancedInputComponent->BindAction(DeadAction,ETriggerEvent::Triggered,this,&ThisClass::OnCharacterDead);
	}
}

void AAInGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

UAbilitySystemComponent* AAInGameCharacter::GetAbilitySystemComponent() const
{
	if (const AAInGamePlayerState* PS = GetPlayerState<AAInGamePlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

void AAInGameCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (GetPlayerState() && GetNetMode() == NM_Client)
	{
		TryInitializeClientASC();
	}
}

void AAInGameCharacter::ARPG_CharacterEquipWeapon(UARPGEventData_Weapon* EventData)
{
	if (EventData->EquipWeaponType == EWeaponType::Sword)
	{
		if (const auto CharAttackComp = GetCharacterAttackComponent())
		{
			CharAttackComp->Server_InitCharacterEquipmentAbility(EventData->WeaponID);
			CharAttackComp->SetOwningWeapon(EventData->WeaponActor);
		}
	}
	else
	{
		if (const auto CharDefenseComp = GetCharacterShieldComponent())
		{
			CharDefenseComp->Server_InitCharacterEquipmentAbility(EventData->WeaponID);
			CharDefenseComp->SetOwningWeapon(EventData->WeaponActor);
		}
	}
}

void AAInGameCharacter::ARPG_CharacterUnequipWeapon(UARPGEventData_Weapon* EventData)
{
	if (EventData->EquipWeaponType == EWeaponType::Sword)
	{
		if (const auto CharAttackComp = GetCharacterAttackComponent())
		{
			CharAttackComp->Server_RemoveCharacterEquipmentAbility();
			CharAttackComp->SetOwningWeapon(nullptr);
			CharAttackComp->Server_InitCharacterEquipmentAbility(FString("W00"));
		}
	}
	else
	{
		if (const auto CharDefenseComp = GetCharacterShieldComponent())
		{
			CharDefenseComp->Server_RemoveCharacterEquipmentAbility();
			CharDefenseComp->SetOwningWeapon(nullptr);
		}
	}
}

void AAInGameCharacter::OnRolling()
{
	// auto CurrentSprintState = CachedSprintComp->GetCurrentState();
	// GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Red,FString::Printf(TEXT("Current Sprint State:%d"),CurrentSprintState));
	if (IsValid(CachedRollComp))
	{
		//GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Red,TEXT("OnRolling"));
		ISpecialOperation::Execute_TriggeredOperation(CachedRollComp);
	}
}

void AAInGameCharacter::OnCharacterAttack()
{
	if (!IsValid(CachedAttackComp)) return;

	CachedAttackComp->Server_CharacterHandleAttack();
}

void AAInGameCharacter::OnCharacterDefenseStart()
{
	if (!IsValid(CachedShieldComp)) return;
	
	ISpecialOperation::Execute_StartedOperation(CachedShieldComp);
}

void AAInGameCharacter::OnCharacterDefenseEnd()
{
	if (!IsValid(CachedShieldComp)) return;
	
	ISpecialOperation::Execute_CompletedOperation(CachedShieldComp,false);
}

void AAInGameCharacter::OnCharacterDead_Implementation()
{

	if (const auto DamageComp = GetCharacterDamageComponent())
	{
		FGameplayTag DamageTag = UGameplayTagsManager::Get().RequestGameplayTag(FName("CharacterAttribute.Health.Consume"));
		FGameplayEventData DamageEvent;
		DamageEvent.EventMagnitude = 10000.f;
		DamageComp->Server_SendApplyDamageEventToActor(this,DamageTag,DamageEvent);
	}
	
}

void AAInGameCharacter::CharacterDeath()
{
	Super::CharacterDeath();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->DisableInput(PC);
	}
	
	if (UWorld* World = GetWorld())
	{
		AInGameMode* GM = World->GetAuthGameMode<AInGameMode>();
		if (GM)
		{
			GM->RequestRespawn(GetController(), 2.0f);
		}
	}
}

UPlayerTargetLockComp* AAInGameCharacter::GetPlayerTargetLockComp()
{
	return CachedPlayerTargetLockComp;
}

void AAInGameCharacter::OnStopRunning()
{
	if (IsValid(CachedSprintComp))
	{
		ISpecialOperation::Execute_CompletedOperation(CachedSprintComp,false);
	}
}

void AAInGameCharacter::OnStartRunning()
{
	if (IsValid(CachedSprintComp))
	{
		ISpecialOperation::Execute_StartedOperation(CachedSprintComp);
	}
}

const UInGameCharacterAttributeSet* AAInGameCharacter::GetAttributeSet() const
{
	if (const AAInGamePlayerState* PS = GetPlayerState<AAInGamePlayerState>())
	{
		return PS->GetAttributeSet();
	}
	return nullptr;
}

void AAInGameCharacter::TryInitializeClientASC()
{
	AAInGamePlayerState* PS = GetPlayerState<AAInGamePlayerState>();
	
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryInitializeClientASC: PlayerState is null."));
		return;
	}
    
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryInitializeClientASC: ASC is null."));
		return;
	}

	SetupAndInitializeASC(PS,ASC);
}

void AAInGameCharacter::SetupAndInitializeASC(AAInGamePlayerState* InGamePlayerState,UAbilitySystemComponent* ASC)
{
	ASC->InitAbilityActorInfo(InGamePlayerState, this);
	
	InGamePlayerState->InitializeAttributesForClient(this);
}
