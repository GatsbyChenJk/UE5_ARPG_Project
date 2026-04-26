#include "UStaminaManagerComponent.h"

#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "ARPGScripts/Gameplay/Combat/AInGamePlayerState.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"

UStaminaManagerComponent::UStaminaManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStaminaManagerComponent::Server_InitStaminaManagerComponent_Implementation()
{
	if (IsValid(OwnerChar))
	{
		OwnerASC = OwnerChar->GetAbilitySystemComponent();
		if (IsValid(OwnerASC))
		{
			OwnerASC->GetGameplayAttributeValueChangeDelegate(UInGameCharacterAttributeSet::GetStaminaAttribute())
					.AddUObject(this,&ThisClass::OnStaminaAttributeChanged);
		}
	}
}

void UStaminaManagerComponent::RemoveActiveRecoverStaminaGE()
{
	OwnerASC->RemoveActiveEffectsWithTags(FGameplayTagContainer(RecoverTag));
}

void UStaminaManagerComponent::RemoveActiveConsumeStaminaGE()
{
	OwnerASC->RemoveActiveEffectsWithTags(FGameplayTagContainer(ConsumeTag));
}

void UStaminaManagerComponent::ActivateRecoverStaminaGE()
{
	// only call on server
	if (!GetOwner()->HasAuthority()) return;
	if (IsValid(StaminaRecoveryEffectClass))
	{
		FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle RecoverSpec = OwnerASC->MakeOutgoingSpec(StaminaRecoveryEffectClass,1,EffectContext);
		if (RecoverSpec.IsValid())
		{
			StaminaRecoveryEffectSpec = OwnerASC->ApplyGameplayEffectSpecToSelf(*RecoverSpec.Data.Get());
		}
	}
}

void UStaminaManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerChar = Cast<AARPGBaseCharacter>(GetOwner());
}

bool UStaminaManagerComponent::IsValidStamina()
{
	if (const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(OwnerChar->GetAttributeSet()))
	{
		bool HasStaminaValue = false;
		float StaminaData = OwnerASC->GetGameplayAttributeValue(AttrSet->GetStaminaAttribute(),HasStaminaValue);
		if (HasStaminaValue)
		{
			return StaminaData > 0.0f;
		}
	}
	return false;
}

void UStaminaManagerComponent::UpdateStaminaUI(const FOnAttributeChangeData& Data, float MaxStamina)
{
	UARPGEventData_OnCharacterAttributeChanged* EventData = NewObject<UARPGEventData_OnCharacterAttributeChanged>();
	EventData->InAttributeCount = Data.NewValue;
	EventData->AttributeBound = MaxStamina;
	ARPG_EVENT_UOBJECT(FName("OnStaminaChanged"), EventData);
}

void UStaminaManagerComponent::OnStaminaAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.0f)
	{
		RemoveActiveConsumeStaminaGE();
	}
	
	if (const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(OwnerChar->GetAttributeSet()))
	{
		float MaxStamina = AttrSet->GetMaxStamina();
		
		if (Data.NewValue >= MaxStamina && StaminaRecoveryEffectSpec.IsValid())
		{
			RemoveActiveRecoverStaminaGE();
		}
		
		//UpdateStaminaUI(Data, MaxStamina);
		//TODO:Update AI Stamina Data in blackboard
	}
}
