// Fill out your copyright notice in the Description page of Project Settings.

#include "ARPGAT_PlayMontageForMeshAndWaitForEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimInstance.h"
#include "ARPGScripts/Gameplay/Base/AbilitySystem/ARPGAbilitySystemComponent.h"
#include "GameFramework/Character.h"

UARPGAT_PlayMontageForMeshAndWaitForEvent::UARPGAT_PlayMontageForMeshAndWaitForEvent(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Rate = 1.f;
	bStopWhenAbilityEnds = true;
	bReplicateMontage = true;
	OverrideBlendOutTimeForCancelAbility = -1.f;
	OverrideBlendOutTimeForStopWhenEndAbility = -1.f;
}

UARPGAbilitySystemComponent* UARPGAT_PlayMontageForMeshAndWaitForEvent::GetTargetASC()
{
	return Cast<UARPGAbilitySystemComponent>(AbilitySystemComponent.Get());
}

void UARPGAT_PlayMontageForMeshAndWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage,
	bool bInterrupted)
{
	if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
	{
		if (Montage == MontageToPlay)
		{
			AbilitySystemComponent.Get()->ClearAnimatingAbility(Ability);

			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy &&
				 Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}
		}
	}

	if (bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UARPGAT_PlayMontageForMeshAndWaitForEvent::OnAbilityCancelled()
{
	if (StopPlayingMontage(OverrideBlendOutTimeForCancelAbility))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UARPGAT_PlayMontageForMeshAndWaitForEvent::OnMontageEnded(UAnimMontage* Montage,
	bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	EndTask();
}

void UARPGAT_PlayMontageForMeshAndWaitForEvent::OnGameplayEvent(FGameplayTag EventTag,
	const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = *Payload;
		TempData.EventTag = EventTag;
		EventReceived.Broadcast(EventTag, TempData);
	}
}

UARPGAT_PlayMontageForMeshAndWaitForEvent*
UARPGAT_PlayMontageForMeshAndWaitForEvent::PlayMontageForMeshAndWaitForEvent(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, USkeletalMeshComponent* InMesh,
	UAnimMontage* InMontageToPlay, FGameplayTagContainer InEventTags, float InRate,
	FName InStartSection, bool bInStopWhenAbilityEnds, float InAnimRootMotionTranslationScale,
	bool bInReplicateMontage, float InOverrideBlendOutTimeForCancelAbility,
	float InOverrideBlendOutTimeForStopWhenEndAbility)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(InRate);

	UARPGAT_PlayMontageForMeshAndWaitForEvent* MyObj =
		NewAbilityTask<UARPGAT_PlayMontageForMeshAndWaitForEvent>(OwningAbility, TaskInstanceName);
	MyObj->Mesh = InMesh;
	MyObj->MontageToPlay = InMontageToPlay;
	MyObj->EventTags = InEventTags;
	MyObj->Rate = InRate;
	MyObj->StartSection = InStartSection;
	MyObj->AnimRootMotionTranslationScale = InAnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bInStopWhenAbilityEnds;
	MyObj->bReplicateMontage = bInReplicateMontage;
	MyObj->OverrideBlendOutTimeForCancelAbility = InOverrideBlendOutTimeForCancelAbility;
	MyObj->OverrideBlendOutTimeForStopWhenEndAbility = InOverrideBlendOutTimeForStopWhenEndAbility;

	return MyObj;
}

void UARPGAT_PlayMontageForMeshAndWaitForEvent::Activate()
{
	if (!Ability)
	{
		return;
	}

	if (!Mesh)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: invalid Mesh"), *FString(__FUNCTION__));
		return;
	}

	bool bPlayedMontage = false;
	UARPGAbilitySystemComponent* ARPGASC = GetTargetASC();

	if (ARPGASC)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			// Bind gameplay event delegate
			EventHandle = ARPGASC->AddGameplayEventTagContainerDelegate(
				EventTags,
				FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
					this, &UARPGAT_PlayMontageForMeshAndWaitForEvent::OnGameplayEvent));

			if (ARPGASC->PlayMontageForMesh(Ability, Mesh, Ability->GetCurrentActivationInfo(),
				MontageToPlay, Rate, StartSection, bReplicateMontage) > 0.f)
			{
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(
					this, &UARPGAT_PlayMontageForMeshAndWaitForEvent::OnAbilityCancelled);

				BlendingOutDelegate.BindUObject(
					this, &UARPGAT_PlayMontageForMeshAndWaitForEvent::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(
					this, &UARPGAT_PlayMontageForMeshAndWaitForEvent::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy &&
					 Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("ARPGAT_PlayMontageForMeshAndWaitForEvent: no AnimInstance on Mesh %s"),
				*GetNameSafe(Mesh));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ARPGAT_PlayMontageForMeshAndWaitForEvent: invalid UARPGAbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ARPGAT_PlayMontageForMeshAndWaitForEvent: Ability %s failed to play montage %s; Task %s."),
			*Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	SetWaitingOnAvatar();
}

void UARPGAT_PlayMontageForMeshAndWaitForEvent::ExternalCancel()
{
	check(AbilitySystemComponent.IsValid());
	OnAbilityCancelled();
	Super::ExternalCancel();
}

void UARPGAT_PlayMontageForMeshAndWaitForEvent::OnDestroy(bool AbilityEnded)
{
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage(OverrideBlendOutTimeForStopWhenEndAbility);
		}
	}

	UARPGAbilitySystemComponent* ARPGASC = GetTargetASC();
	if (ARPGASC)
	{
		ARPGASC->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

bool UARPGAT_PlayMontageForMeshAndWaitForEvent::StopPlayingMontage(float OverrideBlendOutTime)
{
	if (!Mesh)
	{
		return false;
	}

	UARPGAbilitySystemComponent* ARPGASC = GetTargetASC();
	if (!ARPGASC)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return false;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	if (ARPGASC->GetAnimatingAbilityFromAnyMesh() == Ability
		&& ARPGASC->GetCurrentMontageForMesh(Mesh) == MontageToPlay)
	{
		FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
		if (MontageInstance)
		{
			MontageInstance->OnMontageBlendingOutStarted.Unbind();
			MontageInstance->OnMontageEnded.Unbind();
		}

		ARPGASC->CurrentMontageStopForMesh(Mesh, OverrideBlendOutTime);
		return true;
	}

	return false;
}

FString UARPGAT_PlayMontageForMeshAndWaitForEvent::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability && Mesh)
	{
		UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay)
				? MontageToPlay
				: AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(
		TEXT("PlayMontageForMeshAndWaitForEvent. MontageToPlay: %s (Currently Playing): %s"),
		*GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}
