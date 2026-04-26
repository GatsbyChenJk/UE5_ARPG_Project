// Fill out your copyright notice in the Description page of Project Settings.


#include "AInGamePlayerState.h"

#include "InGamePlayerConfig.h"
#include "UInGameCharacterAttributeSet.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Base/SaveGameSystem/ARPGSaveGameManager.h"
#include "ARPGScripts/Gameplay/Character/EAbilityInputID.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "ARPGScripts/Gameplay/Character/InGame/CharacterDamageComponent.h"
#include "ARPGScripts/Gameplay/Character/InGame/HealthManager/CharacterHealthManager.h"
#include "ARPGScripts/Gameplay/Character/InGame/StaminaManager/UStaminaManagerComponent.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/RollComponent.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterAttackComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/SprintComponent.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterShieldComponent.h"

AAInGamePlayerState::AAInGamePlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true); // 启用复制
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UInGameCharacterAttributeSet>(TEXT("AttributeSet"));
	SetNetUpdateFrequency(30.0f);
}

void AAInGamePlayerState::InitializeAttributesForClient(ACharacter* InGameCharacter)
{
	if (!InGameCharacter || !AbilitySystemComponent || HasAuthority())
	{
		return;
	}
	
	const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(AbilitySystemComponent->GetAttributeSet(UInGameCharacterAttributeSet::StaticClass()));
	AttributeSet = AttrSet;
	// for debug use
	// if (AttrSet)
	// {
	// 	// set debug info here
	// 	
	// 	// UARPGEventData_OnCharacterAttributeChanged* EventData = NewObject<UARPGEventData_OnCharacterAttributeChanged>();
	// 	// EventData->InAttributeCount = AttrSet->GetHealth();
	// 	// EventData->AttributeBound = AttrSet->GetMaxHealth();
	// 	// ARPG_EVENT_SIMPLE(FName("OnHealthChanged"),EventData);
	// 	//
	// 	// EventData->InAttributeCount = AttrSet->GetStamina();
	// 	// EventData->AttributeBound = AttrSet->GetMaxStamina();
	// 	// ARPG_EVENT_SIMPLE(FName("OnStaminaChanged"),EventData);
	// }
}

void AAInGamePlayerState::ApplySpecialAbilitiesFromComponents(FCharacterData CharData, AAInGameCharacter* PlayerChar)
{
	// damage
	if (const auto DamageComp = PlayerChar->GetCharacterDamageComponent())
	{
		DamageComp->Server_SetCharacterGetDamage(CharData);
	}

	if (const auto HealthMgrComp = PlayerChar->GetHealthManagerComponent())
	{
		HealthMgrComp->Server_InitHealthManagerComponent();
	}
		
	// special ops
	if (const auto StaminaMgrComp = PlayerChar->GetStaminaManagerComponent())
	{
		StaminaMgrComp->Server_InitStaminaManagerComponent();	
	}
		
	if (const auto SprintComp = PlayerChar->GetSprintComponent())
	{
		//SprintComp->Server_SetCharacterSprintAbility(FString("OP00"));
		SprintComp->Server_SetCharacterSprintAbility(IDConfigDataAsset->CharacterIDConfigs.SprintAbilityID);
	}
		
	if (const auto RollComp = PlayerChar->GetRollComponent())
	{
		//RollComp->Server_SetCharacterRollAbility(FString("OP01"));
		RollComp->Server_SetCharacterRollAbility(IDConfigDataAsset->CharacterIDConfigs.RollAbilityID);
	}
	// ------------------------------------------------------------
		
	// use default weapon as character weapon
	if (const auto AttackComp = PlayerChar->GetCharacterAttackComponent())
	{
		//AttackComp->Server_SetCharacterAttackAbility(FString("W00"));
		AttackComp->Server_InitCharacterEquipmentAbility(IDConfigDataAsset->CharacterIDConfigs.AttackAbilityID);
	}
}

void AAInGamePlayerState::InitializeASCForPlayer(AController* OwningController, APawn* AvatarPawn)
{
	if (GetLocalRole() != ROLE_Authority) return;
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, AvatarPawn);
	}
	
	UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance());
	if (GameInstance && OwningController)
	{
		FCharacterData CharData = GameInstance->GetCharacterData();
		if (!CharData.GetCharacterManifest())
		{
			CharData = GameInstance->GetCharacterDataByID(IDConfigDataAsset->CharacterIDConfigs.DefaultCharacterID);
		}
		
		ApplyInitialAttributesFromData(CharData);
		
		AAInGameCharacter* PlayerChar = Cast<AAInGameCharacter>(GetPawn());
		ApplySpecialAbilitiesFromComponents(CharData, PlayerChar);
		
		OnRep_SetASCInitializedOnServer();
	}
}

void AAInGamePlayerState::ResetPlayer()
{
	if (!IsValid(AbilitySystemComponent)) return;

	AbilitySystemComponent->RemoveActiveEffects(FGameplayEffectQuery());
	
	const auto InGameAttributeSet = GetAttributeSet();
	if (IsValid(InGameAttributeSet))
	{
		AbilitySystemComponent->SetNumericAttributeBase(InGameAttributeSet->GetHealthAttribute(),0.0f);
		AbilitySystemComponent->SetNumericAttributeBase(InGameAttributeSet->GetMaxHealthAttribute(),0.0f);
		AbilitySystemComponent->SetNumericAttributeBase(InGameAttributeSet->GetStaminaAttribute(),0.0f);
		AbilitySystemComponent->SetNumericAttributeBase(InGameAttributeSet->GetMaxStaminaAttribute(),0.0f);
		AbilitySystemComponent->SetNumericAttributeBase(InGameAttributeSet->GetAttackPowerAttribute(),0.0f);
	}
}

UAbilitySystemComponent* AAInGamePlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAInGamePlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAInGamePlayerState, bInitialized);
}

void AAInGamePlayerState::CharacterAbilityInitOnServer(FCharacterData InData)
{
	TSubclassOf<UGameplayAbility> DefaultAbilityClass = InData.GetCharacterManifest()->CharacterMainAbility;

	AbilitySystemComponent->ClearAllAbilities();
	
		FGameplayAbilitySpec AttackSpec(
        		DefaultAbilityClass,                
        		1,                                  
        		EAbilityInputBinds::EInputTypes::DefaultSkill, 
        		this                                
        	);
	
	
	FGameplayAbilitySpecHandle AttackHandle = AbilitySystemComponent->GiveAbility(AttackSpec);
	GrantedAbilities.Add(AttackHandle);
	
}

void AAInGamePlayerState::OnRep_SetASCInitializedOnServer()
{
	bInitialized = true;
}

void AAInGamePlayerState::CharacterAttributeBuffInitOnServer(FCharacterData InData)
{
	TSubclassOf<UGameplayEffect> BuffEffect = InData.GetCharacterManifest()->CharacterBuffEffect;
	if (!IsValid(BuffEffect)) return;
	
	if (auto SaveMgr = GetGameInstance()->GetSubsystem<UARPGSaveGameManager>())
	{
		auto PlayerData = SaveMgr->GetPlayerData();
		
		// get setbycaller tags and value
		TArray<FGameplayTag> ModTags;
		TArray<float> Values;
		ModTags = IDConfigDataAsset->CharacterIDConfigs.BuffTags;

		Values.Add(PlayerData.AttackPowerMod);
		Values.Add(PlayerData.DefensePowerMod);
		Values.Add(PlayerData.HealthMod);
		
		// apply with modifier
		if (Tags.Num() == Values.Num())
		{
			ApplyGE_WithModifier(BuffEffect,ModTags,Values);
		}
	}
}

void AAInGamePlayerState::ApplyGE_NoModifier(TSubclassOf<UGameplayEffect> InEffect)
{
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
    
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		InEffect, 
		1.0f, 
		EffectContext
	);
    
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveEffectHandle = 
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            
		UE_LOG(LogTemp, Log, TEXT("Applied initial attributes GE. Handle: %s"), 
		       *ActiveEffectHandle.ToString());
	}
}

void AAInGamePlayerState::ApplyGE_WithModifier(TSubclassOf<UGameplayEffect> InEffect, TArray<FGameplayTag> InTags,
	TArray<float> InMagnitude)
{
	if (!IsValid(InEffect)) return;
	
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
    
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		InEffect, 1.0f, EffectContext
	);
    
	if (SpecHandle.IsValid())
	{
		for (int i = 0; i < InTags.Num(); ++i)
		{
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle,InTags[i],InMagnitude[i]);
		}
		
		FActiveGameplayEffectHandle ActiveEffectHandle = 
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AAInGamePlayerState::CharacterAttributesInitOnServer(FCharacterData InData)
{
	TSubclassOf<UGameplayEffect> InitEffect = InData.GetCharacterManifest()->CharacterInitData;
	ApplyGE_NoModifier(InitEffect);
}

void AAInGamePlayerState::ApplyInitialAttributesFromData(FCharacterData CharacterData)
{
	if (!AbilitySystemComponent || !HasAuthority())
    {
        return;
    }

	CharacterAttributesInitOnServer(CharacterData);

	CharacterAttributeBuffInitOnServer(CharacterData);
	
	CharacterAbilityInitOnServer(CharacterData);
}
