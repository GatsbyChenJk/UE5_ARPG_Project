// Fill out your copyright notice in the Description page of Project Settings.


#include "RollComponent.h"

#include "FOperationManifest.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Character/EAbilityInputID.h"
#include "ARPGScripts/Gameplay/Character/InGame/StaminaManager/UStaminaManagerComponent.h"
#include "Net/UnrealNetwork.h"


URollComponent::URollComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void URollComponent::TriggeredOperation_Implementation()
{
	Server_ActivateRollAbility();
}

void URollComponent::InitOperation_Implementation(AARPGBaseCharacter* InOwnerCharacter, UAbilitySystemComponent* ASC)
{
	OwnerCharacter = InOwnerCharacter;
	OwnerASC = ASC;
	
}

void URollComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URollComponent, StaminaCost);
	DOREPLIFETIME(URollComponent,CurrentRollWindow);
	DOREPLIFETIME(URollComponent,RollWindow);
	DOREPLIFETIME(URollComponent,bIsRolling);
}

void URollComponent::Server_SetCharacterRollAbility_Implementation(const FString& InOperationID)
{
	OwnerCharacter = Cast<AARPGBaseCharacter>(GetOwner());
	if (IsValid(OwnerCharacter))
	{
		OwnerASC = OwnerCharacter->GetAbilitySystemComponent();
	}
	
	if (IsValid(OwnerASC))
	{
		if (!RollAbilityHandle.IsValid())
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

						RollAbilityHandle = OperationHandle;

						StaminaCost = Manifest.OperationStaminaCost;

						RollWindow = Manifest.StaminaRecoverWindow;
					}
				}
			}
		}

		Execute_InitOperation(this,OwnerCharacter,OwnerASC);
	}
}

void URollComponent::Server_ActivateRollAbility_Implementation()
{
	if (IsValid(OwnerASC))
	{
		if (IsValid(OwnerCharacter))
		{
			if (const auto StaminaComp = OwnerCharacter->GetStaminaManagerComponent())
			{
				if (!StaminaComp->IsValidStamina()) return;
				OwnerASC->TryActivateAbility(RollAbilityHandle);
				StaminaComp->RemoveActiveRecoverStaminaGE();
			}
		}
	}
}

void URollComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AARPGBaseCharacter>(GetOwner());
}

void URollComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickRollWindow(DeltaTime);
}

void URollComponent::ResetRollState()
{
	CurrentRollWindow = 0.f;
	bIsRolling = false;

	if (IsValid(OwnerCharacter))
	{
		if (const auto StaminaComp = OwnerCharacter->GetStaminaManagerComponent())
		{
			StaminaComp->ActivateRecoverStaminaGE();
		}
	}
}

void URollComponent::TickRollWindow(float dt)
{
	if (CurrentRollWindow > 0.f && bIsRolling)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red,FString::Printf(TEXT("Current Roll Window:%f"), CurrentRollWindow));
		CurrentRollWindow -= dt;
		if (CurrentRollWindow <= 0.f)
		{
			ResetRollState();
		}
	}
}






