// Fill out your copyright notice in the Description page of Project Settings.


#include "SprintComponent.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "FOperationManifest.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "ARPGScripts/Gameplay/Character/EAbilityInputID.h"
#include "ARPGScripts/Gameplay/Character/InGame/StaminaManager/UStaminaManagerComponent.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


USprintComponent::USprintComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void USprintComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AARPGBaseCharacter>(GetOwner());
	
}

void USprintComponent::Server_SetCharacterSprintAbility_Implementation(const FString& InOperationID)
{
	OwnerCharacter = Cast<AARPGBaseCharacter>(GetOwner());
	if (IsValid(OwnerCharacter))
	{
		OwnerASC = OwnerCharacter->GetAbilitySystemComponent();
	}
	
	if (IsValid(OwnerASC))
	{
		if (!SprintAbilityHandle.IsValid())
		{
			if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetOwner()->GetGameInstance()))
			{
				FOperationManifest Manifest = GameInstance->GetOperationManifestByID(InOperationID);
				if (!Manifest.OperationAbilities.IsEmpty())
				{
					for (auto& Ability : Manifest.OperationAbilities)
					{
						FGameplayAbilitySpec OperationAbilitySpec(
							Ability,
							1,
							EAbilityInputBinds::EInputTypes::None,
							this);

						FGameplayAbilitySpecHandle OperationHandle = OwnerASC->GiveAbility(OperationAbilitySpec);
						SprintAbilityHandle = OperationHandle;
					}
				}
			}
		}

		Execute_InitOperation(this,OwnerCharacter,OwnerASC);
	}
}

void USprintComponent::OnStaminaChanged(const FOnAttributeChangeData& Data)
{ 
	if (Data.NewValue <= 0.f && CurrentState == ECharacterOperationState::Active)
	{
		UpdateRunMovementParams(false);
		OwnerASC->RemoveActiveGameplayEffect(SprintConsumeEffectHandle);
	}
}

void USprintComponent::InitOperation_Implementation(AARPGBaseCharacter* InOwnerCharacter, UAbilitySystemComponent* ASC)
{
	OwnerCharacter = InOwnerCharacter;
	OwnerASC = ASC;
	
	if (OwnerASC && InOwnerCharacter)
	{
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UInGameCharacterAttributeSet::GetStaminaAttribute())
		.AddUObject(this,&ThisClass::OnStaminaChanged);
		
	}
}

void USprintComponent::UpdateRunMovementParams(bool bIsRunning)
{
	if (!OwnerCharacter) return;
	UCharacterMovementComponent* CharMovement = OwnerCharacter->GetCharacterMovement();
	if (!CharMovement) return;

	if (bIsRunning)
	{
		CharMovement->MaxWalkSpeed = OriginalWalkSpeed * RunSpeedMultiplier;
		CurrentState = ECharacterOperationState::Active;
	}
	else
	{
		CharMovement->MaxWalkSpeed = OriginalWalkSpeed;
		CurrentState = ECharacterOperationState::Inactive;
	}
}

void USprintComponent::Server_ActivateSprintAbility_Implementation(bool bIsSprinting)
{
	if (IsValid(OwnerASC))
	{
		UpdateRunMovementParams(bIsSprinting);
		if (bIsSprinting)
		{
			OwnerASC->TryActivateAbility(SprintAbilityHandle);

			// stop recovering stamina when sprinting
			if (const auto StaminaComp = OwnerCharacter->GetStaminaManagerComponent())
			{
				StaminaComp->RemoveActiveRecoverStaminaGE();
			}
		}
	}
}

void USprintComponent::Server_OnSprintCompleted_Implementation()
 {
 	// TODO : remove consume GE , call stamina comp to start recover GE
 	OwnerASC->RemoveActiveGameplayEffect(SprintConsumeEffectHandle);
 	if (IsValid(OwnerCharacter))
 	{
 		if (const auto StaminaComp = OwnerCharacter->GetStaminaManagerComponent())
 		{
 			StaminaComp->ActivateRecoverStaminaGE();
 		}
 	}
 }

bool USprintComponent::StartedOperation_Implementation()
{
	if (CurrentState != ECharacterOperationState::Inactive) return false;
	
	const UInGameCharacterAttributeSet* AttrSet = OwnerCharacter->GetAttributeSet();
	if (!AttrSet || AttrSet->GetStamina() <= 0.f) return false;
	
	UpdateRunMovementParams(true);
	Server_ActivateSprintAbility(true);

	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Character start running (Component)"));
	return true;
}

void USprintComponent::CompletedOperation_Implementation(bool bIsInterrupted)
{
	if (CurrentState != ECharacterOperationState::Active) return;
	
	UpdateRunMovementParams(false);
	Server_ActivateSprintAbility(false);
	Server_OnSprintCompleted();
	
	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Character Stop running (Component)"));
}

ECharacterOperationState USprintComponent::GetOperationState_Implementation() const
{
	return CurrentState;
}

FString USprintComponent::GetOperationID_Implementation() const
{
	return OperationID;
}

void USprintComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USprintComponent, CurrentState);
}

