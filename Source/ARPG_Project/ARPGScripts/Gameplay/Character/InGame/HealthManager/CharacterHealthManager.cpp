// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterHealthManager.h"

#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"


UCharacterHealthManager::UCharacterHealthManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCharacterHealthManager::IsValidHealth()
{
	if (const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(OwnerChar->GetAttributeSet()))
	{
		bool HasHealthValue = false;
		float HealthData = OwnerASC->GetGameplayAttributeValue(AttrSet->GetHealthAttribute(),HasHealthValue);
		if (HasHealthValue)
		{
			return HealthData > 0.0f;
		}
	}
	return false;
}

void UCharacterHealthManager::Server_InitHealthManagerComponent_Implementation()
{
	if (IsValid(OwnerChar))
	{
		OwnerASC = OwnerChar->GetAbilitySystemComponent();
		if (IsValid(OwnerASC))
		{
			OwnerASC->GetGameplayAttributeValueChangeDelegate(UInGameCharacterAttributeSet::GetHealthAttribute())
					.AddUObject(this,&ThisClass::OnHealthAttributeChanged);
		}
	}
}

void UCharacterHealthManager::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerChar = Cast<AARPGBaseCharacter>(GetOwner());
}

void UCharacterHealthManager::UpdateHealthUI(const FOnAttributeChangeData& Data, float MaxHealth)
{
	UARPGEventData_OnCharacterAttributeChanged* EventData = NewObject<UARPGEventData_OnCharacterAttributeChanged>();
	EventData->InAttributeCount = Data.NewValue;
	EventData->AttributeBound = MaxHealth;
	ARPG_EVENT_WITH_UOBJECT_TARGET(OwnerChar,FName("OnHealthChanged"), EventData);
}

void UCharacterHealthManager::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(OwnerChar->GetAttributeSet()))
	{
		float MaxHealth = AttrSet->GetMaxHealth();

		if (Data.NewValue > MaxHealth)
		{
			// set character health equal to max health
			FGameplayAttribute HealthAttribute = UInGameCharacterAttributeSet::GetHealthAttribute();
			OwnerASC->SetNumericAttributeBase(HealthAttribute, MaxHealth);
		}
		
	}
}


