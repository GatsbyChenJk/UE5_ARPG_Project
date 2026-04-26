
#include "EquipmentComponent.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameAICharacter.h"
#include "ARPGScripts/Gameplay/Combat/AInGamePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "WeaponBase/ARPGBaseWeapon.h"
#include "GameplayTagContainer.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"

UEquipmentComponent::UEquipmentComponent()
{
	CurrentWeapon = nullptr;
	SetIsReplicatedByDefault(true);
}

void UEquipmentComponent::Server_InitAIEquipmentAbility_Implementation(const FString& WeaponID)
{
	AInGameAICharacter* AI = Cast<AInGameAICharacter>(GetOwner());
	if (UAbilitySystemComponent* ASC = AI->GetAbilitySystemComponent())
	{
		FindAndAddEquipmentAbilities(WeaponID,ASC);
		OwnerASC = ASC;
	}
}

void UEquipmentComponent::Server_InitCharacterEquipmentAbility_Implementation(const FString& WeaponID)
{
	AAInGameCharacter* PlayerChar = Cast<AAInGameCharacter>(GetOwner());
	AAInGamePlayerState* PlayerState = Cast<AAInGamePlayerState>(PlayerChar->GetPlayerState());
	
	if (UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent())
	{
		FindAndAddEquipmentAbilities(WeaponID,ASC);
		OwnerASC = ASC;
	}
}

void UEquipmentComponent::Server_RemoveAIEquipmentAbility_Implementation()
{
	AInGameAICharacter* AI = Cast<AInGameAICharacter>(GetOwner());
	if (UAbilitySystemComponent* ASC = AI->GetAbilitySystemComponent())
	{
		RemoveEquipmentAbility(ASC);
	}
}

void UEquipmentComponent::Server_RemoveCharacterEquipmentAbility_Implementation()
{
	AAInGameCharacter* PlayerChar = Cast<AAInGameCharacter>(GetOwner());
	AAInGamePlayerState* PlayerState = Cast<AAInGamePlayerState>(PlayerChar->GetPlayerState());
	
	if (UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent())
	{
		RemoveEquipmentAbility(ASC);
	}
}

AARPGBaseWeapon* UEquipmentComponent::GetCurrentEquippedWeapon()
{
	if (IsValid(CurrentWeapon))
	{
		return CurrentWeapon;
	}
	return nullptr;
}

void UEquipmentComponent::SetOwningWeapon(AARPGBaseWeapon* InWeapon)
{
	CurrentWeapon = InWeapon;
}

float UEquipmentComponent::GetEquipmentAttackAttribute(AARPGBaseCharacter* OwnerChar)
{
	int Final = 0.f;
	if (!IsValid(OwnerChar)) return Final;
	if (const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(OwnerChar->GetAttributeSet()))
	{
		Final += AttrSet->GetAttackPower();
	}
	return Final;
}

float UEquipmentComponent::GetEquipmentStaminaAttribute(AARPGBaseCharacter* OwnerChar)
{
	int Final = 0.f;
	if (!IsValid(OwnerChar)) return Final;
	if (const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(OwnerChar->GetAttributeSet()))
	{
		Final += AttrSet->GetAttackStaminaCost();
	}
	return Final;
}

float UEquipmentComponent::GetEquipmentDefenseAttribute(AARPGBaseCharacter* OwnerChar)
{
	int Final = 0.f;
	if (!IsValid(OwnerChar)) return Final;
	if (const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(OwnerChar->GetAttributeSet()))
	{
		Final += AttrSet->GetDefensePower();
	}
	return Final;
}

float UEquipmentComponent::GetEquipmentDefenseStaminaAttribute(AARPGBaseCharacter* OwnerChar)
{
	int Final = 0.f;
	if (!IsValid(OwnerChar)) return Final;
	if (const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(OwnerChar->GetAttributeSet()))
	{
		Final += AttrSet->GetDefenseStaminaCost();
	}
	return Final;
}


void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipmentComponent,CurrentWeapon)
}

void UEquipmentComponent::RemoveEquipmentAbility(UAbilitySystemComponent* ASC)
{
	// Clear base abilities from last weapon
	if (!EquipmentSpecHandles.IsEmpty())
	{
		for (FGameplayAbilitySpecHandle AbilityHandle : EquipmentSpecHandles)
		{
			ASC->ClearAbility(AbilityHandle);
		}
		EquipmentSpecHandles.Empty();
	}

	// Clear special abilities from last weapon
	if (!SpecialAttackAbilitySpecMap.IsEmpty())
	{
		for (auto It = SpecialAttackAbilitySpecMap.CreateIterator(); It; ++It)
		{
			ASC->ClearAbility(It->Value);
		}
		SpecialAttackAbilitySpecMap.Empty();
	}
}

void UEquipmentComponent::FindAndAddEquipmentAbilities(const FString& WeaponID, UAbilitySystemComponent* ASC)
{
	if (!IsValid(ASC)) return;
	RemoveEquipmentAbility(ASC);

	// Get WeaponConfig
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetOwner()->GetGameInstance()))
	{
		WeaponConfig = GameInstance->GetWeaponManifest();
		if (WeaponConfig.WeaponName.IsEmpty())
		{
			WeaponConfig = GameInstance->GetWeaponManifestByID(WeaponID);
		}
	}

	// set other config data in subclass
}
