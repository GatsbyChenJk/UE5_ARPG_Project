#include "UInGameCharacterAttributeSet.h"

#include "AInGamePlayerState.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/FARPGEventData.h"
#include "ARPGScripts/Gameplay/Character/AI/AIWidgetComponent.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameAICharacter.h"
#include "Net/UnrealNetwork.h"

void UInGameCharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, DefensePower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, AttackStaminaCost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet, DefenseStaminaCost, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UInGameCharacterAttributeSet,CharacterMoney,COND_None,REPNOTIFY_Always);
}

void UInGameCharacterAttributeSet::SendAttributeChangeEvent(FName EventName,UARPGEventData_OnCharacterAttributeChanged* EventData)
{
	if (const auto PlayerState = Cast<AAInGamePlayerState>(GetOwningActor()))
	{
		if (const auto Target = PlayerState->GetPlayerController())
		{
			ARPG_EVENT_WITH_UOBJECT_TARGET(Target,EventName,EventData);
		}
	}
	// for ai get ai character
	if (const auto AIChar = Cast<AInGameAICharacter>(GetOwningActor()))
	{
		GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::Red,FString::Printf(TEXT("Send Attribute Event:%s to AI"),*EventName.ToString()));
		ARPG_EVENT_WITH_UOBJECT_TARGET(AIChar,EventName,EventData);
	}
}

void UInGameCharacterAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	UARPGEventData_OnCharacterAttributeChanged* EventData = NewObject<UARPGEventData_OnCharacterAttributeChanged>();
	EventData->InAttributeCount = GetHealth();
	EventData->AttributeBound = GetMaxHealth();
	
	SendAttributeChangeEvent(FName("OnHealthChanged"),EventData);
	
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet, Health, OldHealth);
}

void UInGameCharacterAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet, MaxHealth, OldMaxHealth);
}

void UInGameCharacterAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet,AttackPower,OldAttackPower);
}

void UInGameCharacterAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	UARPGEventData_OnCharacterAttributeChanged* EventData = NewObject<UARPGEventData_OnCharacterAttributeChanged>();
	EventData->InAttributeCount = GetStamina();
	EventData->AttributeBound = GetMaxStamina();

	SendAttributeChangeEvent(FName("OnStaminaChanged"),EventData);
	
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet,Stamina,OldStamina);
}

void UInGameCharacterAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet,MaxStamina,OldMaxStamina);
}

void UInGameCharacterAttributeSet::OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet,DefensePower,OldDefensePower);
}

void UInGameCharacterAttributeSet::OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet,DamageReduction,OldDamageReduction);
}

void UInGameCharacterAttributeSet::OnRep_AttackStaminaCost(const FGameplayAttributeData& OldStaminaCost)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet,AttackStaminaCost,OldStaminaCost);
}

void UInGameCharacterAttributeSet::OnRep_DefenseStaminaCost(const FGameplayAttributeData& OldStaminaCost)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet,DefenseStaminaCost,OldStaminaCost);
}

void UInGameCharacterAttributeSet::OnRep_Money(const FGameplayAttributeData& OldMoney)
{
	UARPGEventData_OnCharacterAttributeChanged* EventData = NewObject<UARPGEventData_OnCharacterAttributeChanged>();
	EventData->InAttributeCount = GetCharacterMoney();
	SendAttributeChangeEvent(FName("OnMoneyChanged"),EventData);
	
	GAMEPLAYATTRIBUTE_REPNOTIFY(UInGameCharacterAttributeSet,CharacterMoney,OldMoney);
}

