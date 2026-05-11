// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "ARPGAT_PlayMontageForMeshAndWaitForEvent.generated.h"

class UARPGAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FARPGPlayMontageForMeshAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

/**
 * Combines PlayMontageAndWait + WaitForEvent for UARPGAbilitySystemComponent's
 * multi-mesh montage system. Plays a montage on a specific USkeletalMeshComponent
 * and waits for montage callbacks or gameplay events.
 *
 * Adapted from GASShooter's UGSAT_PlayMontageForMeshAndWaitForEvent.
 */
UCLASS()
class ARPG_PROJECT_API UARPGAT_PlayMontageForMeshAndWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()

public:
	UARPGAT_PlayMontageForMeshAndWaitForEvent(const FObjectInitializer& ObjectInitializer);

	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;
	virtual void OnDestroy(bool AbilityEnded) override;

	/** The montage completely finished playing. */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageForMeshAndWaitForEventDelegate OnCompleted;

	/** The montage started blending out. */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageForMeshAndWaitForEventDelegate OnBlendOut;

	/** The montage was interrupted by another montage. */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageForMeshAndWaitForEventDelegate OnInterrupted;

	/** The ability task was explicitly cancelled by another ability. */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageForMeshAndWaitForEventDelegate OnCancelled;

	/** One of the triggering gameplay events fired. */
	UPROPERTY(BlueprintAssignable)
	FARPGPlayMontageForMeshAndWaitForEventDelegate EventReceived;

	/**
	 * Play a montage on a specific skeletal mesh and wait for it to end or for a gameplay event.
	 *
	 * @param OwningAbility            The ability that owns this task.
	 * @param TaskInstanceName          Override the task instance name.
	 * @param Mesh                      Target skeletal mesh (must be owned by AvatarActor).
	 * @param MontageToPlay             The montage asset to play.
	 * @param EventTags                 Gameplay events matching these tags fire EventReceived. Empty = all events.
	 * @param Rate                      Playback rate.
	 * @param StartSection              Section to start from (NAME_None = default).
	 * @param bStopWhenAbilityEnds      If true, montage is stopped when the ability ends normally.
	 * @param AnimRootMotionTranslationScale  Scale applied to root motion translation.
	 * @param bReplicateMontage         Replicate this montage to simulated proxies.
	 * @param OverrideBlendOutTimeForCancelAbility  BlendOut time when the ability is cancelled (-1 = use montage default).
	 * @param OverrideBlendOutTimeForStopWhenEndAbility  BlendOut time when the ability ends (-1 = use montage default).
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks",
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UARPGAT_PlayMontageForMeshAndWaitForEvent* PlayMontageForMeshAndWaitForEvent(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		USkeletalMeshComponent* Mesh,
		UAnimMontage* MontageToPlay,
		FGameplayTagContainer EventTags,
		float Rate = 1.f,
		FName StartSection = NAME_None,
		bool bStopWhenAbilityEnds = true,
		float AnimRootMotionTranslationScale = 1.f,
		bool bReplicateMontage = true,
		float OverrideBlendOutTimeForCancelAbility = -1.f,
		float OverrideBlendOutTimeForStopWhenEndAbility = -1.f);

private:
	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	UAnimMontage* MontageToPlay;

	UPROPERTY()
	FGameplayTagContainer EventTags;

	UPROPERTY()
	float Rate;

	UPROPERTY()
	FName StartSection;

	UPROPERTY()
	float AnimRootMotionTranslationScale;

	UPROPERTY()
	bool bStopWhenAbilityEnds;

	UPROPERTY()
	bool bReplicateMontage;

	UPROPERTY()
	float OverrideBlendOutTimeForCancelAbility;

	UPROPERTY()
	float OverrideBlendOutTimeForStopWhenEndAbility;

	bool StopPlayingMontage(float OverrideBlendOutTime = -1.f);

	UARPGAbilitySystemComponent* GetTargetASC();

	bool bActivated;

	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnAbilityCancelled();
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle CancelledHandle;
	FDelegateHandle EventHandle;
};
