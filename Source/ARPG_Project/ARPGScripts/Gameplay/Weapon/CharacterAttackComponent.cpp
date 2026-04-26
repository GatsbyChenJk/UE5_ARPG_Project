// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttackComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Character/EAbilityInputID.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "ARPGScripts/Gameplay/Character/InGame/CharacterDamageComponent.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameAICharacter.h"
#include "ARPGScripts/Gameplay/Character/InGame/StaminaManager/UStaminaManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "WeaponBase/ARPGBaseWeapon.h"


UCharacterAttackComponent::UCharacterAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterAttackComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCharacterAttackComponent,CurrentAttackCombo)
	DOREPLIFETIME(UCharacterAttackComponent,CurrentWindow)
	DOREPLIFETIME(UCharacterAttackComponent,bIsAttackCombo)
	DOREPLIFETIME(UCharacterAttackComponent,AttackWindow)
	DOREPLIFETIME(UCharacterAttackComponent,DetectSockets)
}

void UCharacterAttackComponent::Server_CharacterHandleAttack_Implementation()
{
	//GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Green,FString::Printf(TEXT("Current Character Combo:%d"),CurrentAttackCombo));
	if (CurrentAttackCombo == 0)
	{
		Server_CharacterActivateAbility();
		bIsAttackCombo = EquipmentSpecHandles.Num() > 0;
		CurrentAttackCombo++;
	}
	else if (CurrentAttackCombo > 0 && CurrentWindow > 0.f)
	{
		Server_CharacterActivateAbility();
		CurrentAttackCombo = (CurrentAttackCombo + 1) % EquipmentSpecHandles.Num();
	}
	else if (CurrentAttackCombo > 0)
	{
		CurrentWindow = 0;
		CurrentAttackCombo = 0;
		
	}
}

void UCharacterAttackComponent::ActivateAttack_Internal(UAbilitySystemComponent* ASC)
{
	AARPGBaseCharacter* OwnerCharacter = Cast<AARPGBaseCharacter>(GetOwner());
	if (IsValid(OwnerCharacter))
	{
		if (const auto StaminaComp = OwnerCharacter->GetStaminaManagerComponent())
		{
			if (!StaminaComp->IsValidStamina()) return;
			StaminaComp->RemoveActiveRecoverStaminaGE();
		}
	}
	
	if (ASC->TryActivateAbility(EquipmentSpecHandles[CurrentAttackCombo]))
	{
		PendingSocketIndex = CurrentAttackCombo;
	}
}


void UCharacterAttackComponent::Server_CharacterActivateAbility_Implementation()
{
	if (CurrentAttackCombo < EquipmentSpecHandles.Num())
	{
		AAInGameCharacter* PlayerChar = Cast<AAInGameCharacter>(GetOwner());
		if (IsValid(PlayerChar))
		{
			if (UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent())
			{
				ActivateAttack_Internal(ASC);
				return;
			}
		}

		AInGameAICharacter* AIChar = Cast<AInGameAICharacter>(GetOwner());
		if (IsValid(AIChar))
		{
			if (UAbilitySystemComponent* ASC = AIChar->GetAbilitySystemComponent())
			{
				ActivateAttack_Internal(ASC);
				return;
			}
		}
	}
	
}

void UCharacterAttackComponent::Server_GetDetectSocketNameByCurrentCombo_Implementation()
{
	//GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Green,FString::Printf(TEXT("Current Character Combo In GA:%d"),PendingSocketIndex));
	if (PendingSocketIndex == INDEX_NONE) return;
	const FName CurrentDetectSocketName = DetectSockets.IsEmpty() ? FName("None") : DetectSockets[PendingSocketIndex];
	if (const auto InGameChar = Cast<AARPGBaseCharacter>(GetOwner()))
	{
		if (const auto DamageComp = InGameChar->GetCharacterDamageComponent())
		{
			DamageComp->SetDetectSocketName(CurrentDetectSocketName);

			Client_SetDetectSocketName(CurrentDetectSocketName);
		}
	}
}


void UCharacterAttackComponent::Client_SetDetectSocketName_Implementation(const FName& SocketName)
{
	if (const auto InGameChar = Cast<AARPGBaseCharacter>(GetOwner()))
	{
		if (const auto DamageComp = InGameChar->GetCharacterDamageComponent())
		{
			DamageComp->SetDetectSocketName(SocketName);
		}
	}
}

void UCharacterAttackComponent::ActivateSpecialAbilities(UAbilitySystemComponent* ASC,
	EEquipmentActivateType ActivateType)
{
	if (SpecialAttackAbilitySpecMap.FindChecked(ActivateType).IsValid())
	{
		ASC->TryActivateAbility(SpecialAttackAbilitySpecMap[ActivateType]);
	}
}

void UCharacterAttackComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterAttackComponent::ResetComboState()
{
	SetIsActtackCombo(false);
	CurrentWindow = 0.f;
	CurrentAttackCombo = 0;

	AARPGBaseCharacter* OwnerCharacter = Cast<AARPGBaseCharacter>(GetOwner());
	if (IsValid(OwnerCharacter))
	{
		if (const auto StaminaComp = OwnerCharacter->GetStaminaManagerComponent())
		{
			StaminaComp->ActivateRecoverStaminaGE();
		}
	}
}

void UCharacterAttackComponent::TickCurrentWindow(float DeltaTime)
{
	if (CurrentWindow > 0.f && bIsAttackCombo)
	{
		// GEngine->AddOnScreenDebugMessage(-1,0.f,FColor::Green,FString::Printf(TEXT("Current Character Window:%f,Character:%s"),
		// 	CurrentWindow,*GetOwner()->GetName()));
		CurrentWindow -= DeltaTime;
		if (CurrentWindow <= 0.f)
		{
			//reset combo state
			ResetComboState();
		}
	}
	
}

void UCharacterAttackComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickCurrentWindow(DeltaTime);
}

void UCharacterAttackComponent::FindAndAddEquipmentAbilities(const FString& WeaponID, UAbilitySystemComponent* ASC)
{
	Super::FindAndAddEquipmentAbilities(WeaponID, ASC);

	// add attack window
	AttackWindow = WeaponConfig.ComboWindow;
	
	// add and give abilities
	for (int i = 0;i<WeaponConfig.EquipmentBaseAbilityClasses.Num();i++)
	{
		FGameplayAbilitySpec AttackSpec(
			WeaponConfig.EquipmentBaseAbilityClasses[i],1,                                  
			EAbilityInputBinds::None, this
			);

		FGameplayAbilitySpecHandle AttackHandle = ASC->GiveAbility(AttackSpec);
		EquipmentSpecHandles.Add(AttackHandle);
	}

	// add special abilities
	for (auto SpecialAbilityConfig : WeaponConfig.SpecialAbilityConfig)
	{
		FGameplayAbilitySpec SpecialSpec(
			SpecialAbilityConfig.EquipmentAbilityClass,1,
			EAbilityInputBinds::None, this
			);

		FGameplayAbilitySpecHandle SpecialAbilityHandle = ASC->GiveAbility(SpecialSpec);
		SpecialAttackAbilitySpecMap.Add(SpecialAbilityConfig.ActivateType,SpecialAbilityHandle);
	}
	
	// add detect socket
	DetectSockets = WeaponConfig.DetectSocketNames;
}

void UCharacterAttackComponent::Server_CharacterHandleAttackSpecial_Implementation(EEquipmentActivateType ActivateType)
{
	AAInGameCharacter* PlayerChar = Cast<AAInGameCharacter>(GetOwner());
	if (IsValid(PlayerChar))
	{
		if (UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent())
		{
			ActivateSpecialAbilities(ASC, ActivateType);				
			return;
		}
	}

	AInGameAICharacter* AIChar = Cast<AInGameAICharacter>(GetOwner());
	if (IsValid(AIChar))
	{
		if (UAbilitySystemComponent* ASC = AIChar->GetAbilitySystemComponent())
		{
			ActivateSpecialAbilities(ASC, ActivateType);
			return;
		}
	}
	
}



