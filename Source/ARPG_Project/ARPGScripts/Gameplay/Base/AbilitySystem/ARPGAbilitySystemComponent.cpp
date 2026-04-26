// Fill out your copyright notice in the Description page of Project Settings.

#include "ARPGAbilitySystemComponent.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemGlobals.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/ARPGBaseWeapon.h"

static TAutoConsoleVariable<float> CVarARPGMontageErrorThreshold(
	TEXT("ARPG.replay.MontageErrorThreshold"),
	0.5f,
	TEXT("Position correction threshold (seconds) for ARPG multi-mesh montage replay sync.")
);

// ─────────────────────────────────────────────────────────────────────────────
// Construction / Overrides
// ─────────────────────────────────────────────────────────────────────────────

UARPGAbilitySystemComponent::UARPGAbilitySystemComponent()
	: TrackedWeaponActor(nullptr)
{
}

void UARPGAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UARPGAbilitySystemComponent, RepAnimMontageInfoForMeshes);
	DOREPLIFETIME(UARPGAbilitySystemComponent, RepSocketData);
}

bool UARPGAbilitySystemComponent::GetShouldTick() const
{
	// Keep ticking on the server as long as a montage is running (to push net updates)
	for (const FARPGRepAnimMontageForMesh& RepInfo : RepAnimMontageInfoForMeshes)
	{
		if (IsOwnerActorAuthoritative() && !RepInfo.RepMontageInfo.IsStopped)
		{
			return true;
		}
	}

	// Keep ticking on the server when socket replication is active
	if (IsOwnerActorAuthoritative() &&
		(TrackedCharacterSockets.Num() > 0 || TrackedWeaponSockets.Num() > 0))
	{
		return true;
	}

	return Super::GetShouldTick();
}

void UARPGAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	if (IsOwnerActorAuthoritative())
	{
		// Push per-mesh montage replication data to RepAnimMontageInfoForMeshes
		for (FARPGLocalAnimMontageForMesh& MontageInfo : LocalAnimMontageInfoForMeshes)
		{
			AnimMontage_UpdateReplicatedDataForMesh(MontageInfo.Mesh);
		}

		// Update socket replication snapshot
		if (TrackedCharacterSockets.Num() > 0 || TrackedWeaponSockets.Num() > 0)
		{
			Server_UpdateSocketReplicationData();
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UARPGAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	LocalAnimMontageInfoForMeshes.Reset();
	RepAnimMontageInfoForMeshes.Reset();

	// If a montage rep arrived before the avatar actor was ready, process it now
	if (bPendingMontageRep)
	{
		OnRep_ReplicatedAnimMontageForMesh();
	}
}

void UARPGAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle,
	UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);
	ClearAnimatingAbilityForAllMeshes(Ability);
}

// ─────────────────────────────────────────────────────────────────────────────
// Multi-mesh Montage – Play
// ─────────────────────────────────────────────────────────────────────────────

float UARPGAbilitySystemComponent::PlayMontageForMesh(UGameplayAbility* AnimatingAbility,
	USkeletalMeshComponent* InMesh, FGameplayAbilityActivationInfo ActivationInfo,
	UAnimMontage* NewAnimMontage, float InPlayRate, FName StartSectionName, bool bReplicateMontage)
{
	float Duration = -1.f;

	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	if (!AnimInstance || !NewAnimMontage)
	{
		return Duration;
	}

	Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
	if (Duration <= 0.f)
	{
		return Duration;
	}

	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	// If a different ability was animating on this mesh it should have already
	// handled the interrupt; we just overwrite its slot here.
	LocalInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
	LocalInfo.LocalMontageInfo.AnimatingAbility = AnimatingAbility;
	LocalInfo.LocalMontageInfo.PlayInstanceId++;

	if (StartSectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(StartSectionName, NewAnimMontage);
	}

	if (IsOwnerActorAuthoritative())
	{
		if (bReplicateMontage)
		{
			FARPGRepAnimMontageForMesh& RepInfo = GetRepAnimMontageInfoForMesh(InMesh);
			RepInfo.RepMontageInfo.Animation = NewAnimMontage;
			RepInfo.RepMontageInfo.PlayInstanceId++;

			AnimMontage_UpdateReplicatedDataForMesh(InMesh);

			if (AbilityActorInfo->AvatarActor.IsValid())
			{
				AbilityActorInfo->AvatarActor->ForceNetUpdate();
			}
		}
	}
	else
	{
		// Client predictive play: register rejection callback
		FPredictionKey PredictionKey = GetPredictionKeyForNewAction();
		if (PredictionKey.IsValidKey())
		{
			PredictionKey.NewRejectedDelegate().BindUObject(
				this, &UARPGAbilitySystemComponent::OnPredictiveMontageRejectedForMesh,
				InMesh, NewAnimMontage);
		}
	}

	return Duration;
}

float UARPGAbilitySystemComponent::PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh,
	UAnimMontage* NewAnimMontage, float InPlayRate, FName StartSectionName)
{
	float Duration = -1.f;
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	if (!AnimInstance || !NewAnimMontage)
	{
		return Duration;
	}

	Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate);
	if (Duration > 0.f)
	{
		FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);
		LocalInfo.LocalMontageInfo.AnimMontage = NewAnimMontage;
	}

	return Duration;
}

// ─────────────────────────────────────────────────────────────────────────────
// Multi-mesh Montage – Stop / Section / PlayRate
// ─────────────────────────────────────────────────────────────────────────────

void UARPGAbilitySystemComponent::CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh,
	float OverrideBlendOutTime)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	UAnimMontage* MontageToStop = LocalInfo.LocalMontageInfo.AnimMontage;

	if (AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop))
	{
		const float BlendOutTime = (OverrideBlendOutTime >= 0.f)
			? OverrideBlendOutTime
			: MontageToStop->BlendOut.GetBlendTime();

		AnimInstance->Montage_Stop(BlendOutTime, MontageToStop);

		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedDataForMesh(InMesh);
		}
	}
}

void UARPGAbilitySystemComponent::StopAllCurrentMontages(float OverrideBlendOutTime)
{
	for (FARPGLocalAnimMontageForMesh& Info : LocalAnimMontageInfoForMeshes)
	{
		CurrentMontageStopForMesh(Info.Mesh, OverrideBlendOutTime);
	}
}

void UARPGAbilitySystemComponent::StopMontageIfCurrentForMesh(USkeletalMeshComponent* InMesh,
	const UAnimMontage& Montage, float OverrideBlendOutTime)
{
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);
	if (&Montage == LocalInfo.LocalMontageInfo.AnimMontage)
	{
		CurrentMontageStopForMesh(InMesh, OverrideBlendOutTime);
	}
}

void UARPGAbilitySystemComponent::ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability)
{
	for (FARPGLocalAnimMontageForMesh& Info : LocalAnimMontageInfoForMeshes)
	{
		if (Info.LocalMontageInfo.AnimatingAbility == Ability)
		{
			Info.LocalMontageInfo.AnimatingAbility = nullptr;
		}
	}
}

void UARPGAbilitySystemComponent::CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh,
	FName SectionName)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (SectionName == NAME_None || !AnimInstance || !LocalInfo.LocalMontageInfo.AnimMontage)
	{
		return;
	}

	AnimInstance->Montage_JumpToSection(SectionName, LocalInfo.LocalMontageInfo.AnimMontage);

	if (IsOwnerActorAuthoritative())
	{
		AnimMontage_UpdateReplicatedDataForMesh(InMesh);
	}
	else
	{
		ServerCurrentMontageJumpToSectionNameForMesh(InMesh, LocalInfo.LocalMontageInfo.AnimMontage, SectionName);
	}
}

void UARPGAbilitySystemComponent::CurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh,
	FName FromSectionName, FName ToSectionName)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (!AnimInstance || !LocalInfo.LocalMontageInfo.AnimMontage)
	{
		return;
	}

	AnimInstance->Montage_SetNextSection(FromSectionName, ToSectionName, LocalInfo.LocalMontageInfo.AnimMontage);

	if (IsOwnerActorAuthoritative())
	{
		AnimMontage_UpdateReplicatedDataForMesh(InMesh);
	}
	else
	{
		const float CurrentPosition = AnimInstance->Montage_GetPosition(LocalInfo.LocalMontageInfo.AnimMontage);
		ServerCurrentMontageSetNextSectionNameForMesh(InMesh, LocalInfo.LocalMontageInfo.AnimMontage,
			CurrentPosition, FromSectionName, ToSectionName);
	}
}

void UARPGAbilitySystemComponent::CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh,
	float InPlayRate)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (!AnimInstance || !LocalInfo.LocalMontageInfo.AnimMontage)
	{
		return;
	}

	AnimInstance->Montage_SetPlayRate(LocalInfo.LocalMontageInfo.AnimMontage, InPlayRate);

	if (IsOwnerActorAuthoritative())
	{
		AnimMontage_UpdateReplicatedDataForMesh(InMesh);
	}
	else
	{
		ServerCurrentMontageSetPlayRateForMesh(InMesh, LocalInfo.LocalMontageInfo.AnimMontage, InPlayRate);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Multi-mesh Montage – Queries
// ─────────────────────────────────────────────────────────────────────────────

bool UARPGAbilitySystemComponent::IsAnimatingAbilityForAnyMesh(UGameplayAbility* Ability) const
{
	for (const FARPGLocalAnimMontageForMesh& Info : LocalAnimMontageInfoForMeshes)
	{
		if (Info.LocalMontageInfo.AnimatingAbility == Ability)
		{
			return true;
		}
	}
	return false;
}

UGameplayAbility* UARPGAbilitySystemComponent::GetAnimatingAbilityFromAnyMesh()
{
	for (FARPGLocalAnimMontageForMesh& Info : LocalAnimMontageInfoForMeshes)
	{
		if (Info.LocalMontageInfo.AnimatingAbility.IsValid())
		{
			return Info.LocalMontageInfo.AnimatingAbility.Get();
		}
	}
	return nullptr;
}

TArray<UAnimMontage*> UARPGAbilitySystemComponent::GetCurrentMontages() const
{
	TArray<UAnimMontage*> Montages;
	for (const FARPGLocalAnimMontageForMesh& Info : LocalAnimMontageInfoForMeshes)
	{
		UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(Info.Mesh)
			? Info.Mesh->GetAnimInstance() : nullptr;

		if (Info.LocalMontageInfo.AnimMontage && AnimInstance
			&& AnimInstance->Montage_IsActive(Info.LocalMontageInfo.AnimMontage))
		{
			Montages.Add(Info.LocalMontageInfo.AnimMontage);
		}
	}
	return Montages;
}

UAnimMontage* UARPGAbilitySystemComponent::GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (LocalInfo.LocalMontageInfo.AnimMontage && AnimInstance
		&& AnimInstance->Montage_IsActive(LocalInfo.LocalMontageInfo.AnimMontage))
	{
		return LocalInfo.LocalMontageInfo.AnimMontage;
	}
	return nullptr;
}

int32 UARPGAbilitySystemComponent::GetCurrentMontageSectionIDForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	UAnimMontage* CurrentMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentMontage && AnimInstance)
	{
		const float MontagePos = AnimInstance->Montage_GetPosition(CurrentMontage);
		return CurrentMontage->GetSectionIndexFromPosition(MontagePos);
	}
	return INDEX_NONE;
}

FName UARPGAbilitySystemComponent::GetCurrentMontageSectionNameForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	UAnimMontage* CurrentMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentMontage && AnimInstance)
	{
		const float MontagePos = AnimInstance->Montage_GetPosition(CurrentMontage);
		const int32 SectionID = CurrentMontage->GetSectionIndexFromPosition(MontagePos);
		return CurrentMontage->GetSectionName(SectionID);
	}
	return NAME_None;
}

float UARPGAbilitySystemComponent::GetCurrentMontageSectionLengthForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	UAnimMontage* CurrentMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentMontage && AnimInstance)
	{
		const int32 CurrentSectionID = GetCurrentMontageSectionIDForMesh(InMesh);
		if (CurrentSectionID != INDEX_NONE)
		{
			const TArray<FCompositeSection>& Sections = CurrentMontage->CompositeSections;
			if (CurrentSectionID < Sections.Num() - 1)
			{
				return Sections[CurrentSectionID + 1].GetTime() - Sections[CurrentSectionID].GetTime();
			}
			// Last section: measure to end of montage (UE5: GetPlayLength() replaces SequenceLength)
			return CurrentMontage->GetPlayLength() - Sections[CurrentSectionID].GetTime();
		}
		return CurrentMontage->GetPlayLength();
	}
	return 0.f;
}

float UARPGAbilitySystemComponent::GetCurrentMontageSectionTimeLeftForMesh(USkeletalMeshComponent* InMesh)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	UAnimMontage* CurrentMontage = GetCurrentMontageForMesh(InMesh);

	if (CurrentMontage && AnimInstance && AnimInstance->Montage_IsActive(CurrentMontage))
	{
		const float CurrentPos = AnimInstance->Montage_GetPosition(CurrentMontage);
		return CurrentMontage->GetSectionTimeLeftFromPos(CurrentPos);
	}
	return -1.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Multi-mesh Montage – Internal helpers
// ─────────────────────────────────────────────────────────────────────────────

FARPGLocalAnimMontageForMesh& UARPGAbilitySystemComponent::GetLocalAnimMontageInfoForMesh(
	USkeletalMeshComponent* InMesh)
{
	for (FARPGLocalAnimMontageForMesh& Info : LocalAnimMontageInfoForMeshes)
	{
		if (Info.Mesh == InMesh)
		{
			return Info;
		}
	}
	LocalAnimMontageInfoForMeshes.Emplace(InMesh);
	return LocalAnimMontageInfoForMeshes.Last();
}

FARPGRepAnimMontageForMesh& UARPGAbilitySystemComponent::GetRepAnimMontageInfoForMesh(
	USkeletalMeshComponent* InMesh)
{
	for (FARPGRepAnimMontageForMesh& Info : RepAnimMontageInfoForMeshes)
	{
		if (Info.Mesh == InMesh)
		{
			return Info;
		}
	}
	RepAnimMontageInfoForMeshes.Emplace(InMesh);
	return RepAnimMontageInfoForMeshes.Last();
}

void UARPGAbilitySystemComponent::OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh,
	UAnimMontage* PredictiveMontage)
{
	static constexpr float MONTAGE_REJECT_FADETIME = 0.25f;

	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	if (AnimInstance && PredictiveMontage && AnimInstance->Montage_IsPlaying(PredictiveMontage))
	{
		AnimInstance->Montage_Stop(MONTAGE_REJECT_FADETIME, PredictiveMontage);
	}
}

void UARPGAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh)
{
	check(IsOwnerActorAuthoritative());
	AnimMontage_UpdateReplicatedDataForMesh(GetRepAnimMontageInfoForMesh(InMesh));
}

void UARPGAbilitySystemComponent::AnimMontage_UpdateReplicatedDataForMesh(
	FARPGRepAnimMontageForMesh& OutRepInfo)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(OutRepInfo.Mesh)
		? OutRepInfo.Mesh->GetAnimInstance() : nullptr;

	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(OutRepInfo.Mesh);

	if (!AnimInstance || !LocalInfo.LocalMontageInfo.AnimMontage)
	{
		return;
	}

	OutRepInfo.RepMontageInfo.Animation = LocalInfo.LocalMontageInfo.AnimMontage;

	const bool bIsStopped = AnimInstance->Montage_GetIsStopped(LocalInfo.LocalMontageInfo.AnimMontage);
	if (!bIsStopped)
	{
		OutRepInfo.RepMontageInfo.PlayRate  = AnimInstance->Montage_GetPlayRate(LocalInfo.LocalMontageInfo.AnimMontage);
		OutRepInfo.RepMontageInfo.Position  = AnimInstance->Montage_GetPosition(LocalInfo.LocalMontageInfo.AnimMontage);
		OutRepInfo.RepMontageInfo.BlendTime = AnimInstance->Montage_GetBlendTime(LocalInfo.LocalMontageInfo.AnimMontage);
	}

	if (OutRepInfo.RepMontageInfo.IsStopped != bIsStopped)
	{
		OutRepInfo.RepMontageInfo.IsStopped = bIsStopped;

		if (AbilityActorInfo->AvatarActor.IsValid())
		{
			AbilityActorInfo->AvatarActor->ForceNetUpdate();
		}

		UpdateShouldTick();
	}

	// Replicate NextSectionID (stored as index+1 so that 0 means INDEX_NONE)
	const int32 CurrentSectionID = LocalInfo.LocalMontageInfo.AnimMontage
		->GetSectionIndexFromPosition(OutRepInfo.RepMontageInfo.Position);

	if (CurrentSectionID != INDEX_NONE)
	{
		const int32 NextSectionID = AnimInstance->Montage_GetNextSectionID(
			LocalInfo.LocalMontageInfo.AnimMontage, CurrentSectionID);

		if (ensure(NextSectionID < 255))
		{
			OutRepInfo.RepMontageInfo.NextSectionID = uint8(NextSectionID + 1);
		}
	}
	else
	{
		OutRepInfo.RepMontageInfo.NextSectionID = 0;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Multi-mesh Montage – OnRep (simulated proxy side)
// ─────────────────────────────────────────────────────────────────────────────

void UARPGAbilitySystemComponent::OnRep_ReplicatedAnimMontageForMesh()
{
	UWorld* World = GetWorld();
	const bool bIsPlayingReplay = World && World->IsPlayingReplay();
	const float MontageErrorThreshold = bIsPlayingReplay
		? CVarARPGMontageErrorThreshold.GetValueOnGameThread()
		: 0.1f;

	for (FARPGRepAnimMontageForMesh& RepInfo : RepAnimMontageInfoForMeshes)
	{
		FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(RepInfo.Mesh);

		if (RepInfo.RepMontageInfo.bSkipPlayRate)
		{
			RepInfo.RepMontageInfo.PlayRate = 1.f;
		}

		UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(RepInfo.Mesh)
			? RepInfo.Mesh->GetAnimInstance() : nullptr;

		if (!AnimInstance || !IsReadyForReplicatedMontageForMesh())
		{
			bPendingMontageRep = true;
			return;
		}
		bPendingMontageRep = false;

		if (AbilityActorInfo->IsLocallyControlled())
		{
			// Owning client already applied predictions locally; skip.
			continue;
		}

		UAnimMontage* const RepMontage = RepInfo.RepMontageInfo.GetAnimMontage();
		if (!RepMontage)
		{
			continue;
		}

		// ── Start / restart montage if needed ────────────────────────────────
		const uint8 ReplicatedPlayInstanceId = RepInfo.RepMontageInfo.PlayInstanceId;
		if (LocalInfo.LocalMontageInfo.AnimMontage != RepMontage
			|| LocalInfo.LocalMontageInfo.PlayInstanceId != ReplicatedPlayInstanceId)
		{
			LocalInfo.LocalMontageInfo.PlayInstanceId = ReplicatedPlayInstanceId;
			PlayMontageSimulatedForMesh(RepInfo.Mesh, RepMontage,
				RepInfo.RepMontageInfo.PlayRate);
		}

		if (!LocalInfo.LocalMontageInfo.AnimMontage)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("UARPGAbilitySystemComponent::OnRep_ReplicatedAnimMontageForMesh: "
					 "PlayMontageSimulated failed for %s on %s"),
				*GetNameSafe(RepMontage), *GetNameSafe(RepInfo.Mesh));
			continue;
		}

		// ── Sync play rate ────────────────────────────────────────────────────
		if (!FMath::IsNearlyEqual(
			AnimInstance->Montage_GetPlayRate(LocalInfo.LocalMontageInfo.AnimMontage),
			RepInfo.RepMontageInfo.PlayRate))
		{
			AnimInstance->Montage_SetPlayRate(LocalInfo.LocalMontageInfo.AnimMontage,
				RepInfo.RepMontageInfo.PlayRate);
		}

		const bool bLocalIsStopped = AnimInstance->Montage_GetIsStopped(LocalInfo.LocalMontageInfo.AnimMontage);
		const bool bRepIsStopped   = bool(RepInfo.RepMontageInfo.IsStopped);

		// ── Stop first to avoid section-blend pops ───────────────────────────
		if (bRepIsStopped)
		{
			if (!bLocalIsStopped)
			{
				CurrentMontageStopForMesh(RepInfo.Mesh, RepInfo.RepMontageInfo.BlendTime);
			}
			continue;
		}

		// ── Section / position correction ─────────────────────────────────────
		if (!RepInfo.RepMontageInfo.SkipPositionCorrection)
		{
			const int32 RepSectionID     = LocalInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(RepInfo.RepMontageInfo.Position);
			const int32 RepNextSectionID = int32(RepInfo.RepMontageInfo.NextSectionID) - 1;

			if (RepSectionID != INDEX_NONE)
			{
				const int32 LocalNextSectionID = AnimInstance->Montage_GetNextSectionID(
					LocalInfo.LocalMontageInfo.AnimMontage, RepSectionID);

				if (LocalNextSectionID != RepNextSectionID)
				{
					AnimInstance->Montage_SetNextSection(
						LocalInfo.LocalMontageInfo.AnimMontage->GetSectionName(RepSectionID),
						LocalInfo.LocalMontageInfo.AnimMontage->GetSectionName(RepNextSectionID),
						LocalInfo.LocalMontageInfo.AnimMontage);
				}

				// Teleport to correct section if client drifted
				const int32 CurrentSectionID = LocalInfo.LocalMontageInfo.AnimMontage
					->GetSectionIndexFromPosition(AnimInstance->Montage_GetPosition(LocalInfo.LocalMontageInfo.AnimMontage));

				if (CurrentSectionID != RepSectionID && CurrentSectionID != RepNextSectionID)
				{
					const float SectionStartTime = LocalInfo.LocalMontageInfo.AnimMontage
						->GetAnimCompositeSection(RepSectionID).GetTime();
					AnimInstance->Montage_SetPosition(LocalInfo.LocalMontageInfo.AnimMontage, SectionStartTime);
				}
			}

			// Position error correction
			const float LocalPosition  = AnimInstance->Montage_GetPosition(LocalInfo.LocalMontageInfo.AnimMontage);
			const int32 LocalSectionID = LocalInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(LocalPosition);
			const float DeltaPosition  = RepInfo.RepMontageInfo.Position - LocalPosition;

			if (LocalSectionID == RepSectionID
				&& FMath::Abs(DeltaPosition) > MontageErrorThreshold
				&& !RepInfo.RepMontageInfo.IsStopped)
			{
				// Fast-forward to server position, firing intermediate events
				if (FAnimMontageInstance* MontageInstance =
					AnimInstance->GetActiveInstanceForMontage(RepMontage))
				{
					const float DeltaTime = !FMath::IsNearlyZero(RepInfo.RepMontageInfo.PlayRate)
						? DeltaPosition / RepInfo.RepMontageInfo.PlayRate : 0.f;

					if (DeltaTime >= 0.f)
					{
						MontageInstance->UpdateWeight(DeltaTime);
						MontageInstance->HandleEvents(LocalPosition, RepInfo.RepMontageInfo.Position, nullptr);
						AnimInstance->TriggerAnimNotifies(DeltaTime);
					}
				}
				AnimInstance->Montage_SetPosition(LocalInfo.LocalMontageInfo.AnimMontage,
					RepInfo.RepMontageInfo.Position);
			}
		}
	}
}

bool UARPGAbilitySystemComponent::IsReadyForReplicatedMontageForMesh()
{
	// Subclasses may override (e.g. wait for a skin/mesh swap to finish)
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Multi-mesh Montage – Server RPCs (section / play-rate sync from clients)
// ─────────────────────────────────────────────────────────────────────────────

void UARPGAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Implementation(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition,
	FName SectionName, FName NextSectionName)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (!AnimInstance || ClientAnimMontage != LocalInfo.LocalMontageInfo.AnimMontage)
	{
		return;
	}

	AnimInstance->Montage_SetNextSection(SectionName, NextSectionName, LocalInfo.LocalMontageInfo.AnimMontage);

	// Validate position
	const float ServerPosition  = AnimInstance->Montage_GetPosition(LocalInfo.LocalMontageInfo.AnimMontage);
	const int32 ServerSectionID = LocalInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(ServerPosition);
	const int32 ClientSectionID = LocalInfo.LocalMontageInfo.AnimMontage->GetSectionIndexFromPosition(ClientPosition);

	if (LocalInfo.LocalMontageInfo.AnimMontage->GetSectionName(ServerSectionID) != SectionName
		|| ServerSectionID != ClientSectionID)
	{
		AnimInstance->Montage_SetPosition(LocalInfo.LocalMontageInfo.AnimMontage, ClientPosition);
	}

	AnimMontage_UpdateReplicatedDataForMesh(InMesh);
}

bool UARPGAbilitySystemComponent::ServerCurrentMontageSetNextSectionNameForMesh_Validate(
	USkeletalMeshComponent*, UAnimMontage*, float, FName, FName)
{
	return true;
}

void UARPGAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Implementation(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (!AnimInstance || ClientAnimMontage != LocalInfo.LocalMontageInfo.AnimMontage)
	{
		return;
	}

	AnimInstance->Montage_JumpToSection(SectionName, LocalInfo.LocalMontageInfo.AnimMontage);
	AnimMontage_UpdateReplicatedDataForMesh(InMesh);
}

bool UARPGAbilitySystemComponent::ServerCurrentMontageJumpToSectionNameForMesh_Validate(
	USkeletalMeshComponent*, UAnimMontage*, FName)
{
	return true;
}

void UARPGAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Implementation(
	USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float InPlayRate)
{
	UAnimInstance* AnimInstance = IsMeshOwnedByAvatar(InMesh) ? InMesh->GetAnimInstance() : nullptr;
	FARPGLocalAnimMontageForMesh& LocalInfo = GetLocalAnimMontageInfoForMesh(InMesh);

	if (!AnimInstance || ClientAnimMontage != LocalInfo.LocalMontageInfo.AnimMontage)
	{
		return;
	}

	AnimInstance->Montage_SetPlayRate(LocalInfo.LocalMontageInfo.AnimMontage, InPlayRate);
	AnimMontage_UpdateReplicatedDataForMesh(InMesh);
}

bool UARPGAbilitySystemComponent::ServerCurrentMontageSetPlayRateForMesh_Validate(
	USkeletalMeshComponent*, UAnimMontage*, float)
{
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Socket Replication – Public API
// ─────────────────────────────────────────────────────────────────────────────

void UARPGAbilitySystemComponent::SetTrackedCharacterSocketNames(const TArray<FName>& SocketNames)
{
	TrackedCharacterSockets = SocketNames;
	UpdateShouldTick();
}

void UARPGAbilitySystemComponent::SetTrackedWeaponSocketNames(const TArray<FName>& SocketNames)
{
	TrackedWeaponSockets = SocketNames;
	UpdateShouldTick();
}

void UARPGAbilitySystemComponent::SetTrackedWeaponActor(AARPGBaseWeapon* InWeapon)
{
	TrackedWeaponActor = InWeapon;
}

void UARPGAbilitySystemComponent::ClearTrackedWeaponActor()
{
	TrackedWeaponActor = nullptr;

	// Remove stale weapon entries from the replicated array so clients clear them
	RepSocketData.RemoveAll([](const FARPGRepSocketEntry& E) { return E.bIsWeaponSocket; });

	if (AbilityActorInfo.IsValid() && AbilityActorInfo->AvatarActor.IsValid())
	{
		AbilityActorInfo->AvatarActor->ForceNetUpdate();
	}
}

FTransform UARPGAbilitySystemComponent::GetReplicatedSocketTransform(FName SocketName,
	bool bFromWeaponMesh, bool& bFound) const
{
	bFound = false;

	// On the server or autonomous proxy: query the live mesh for maximum accuracy
	if (IsOwnerActorAuthoritative() || AbilityActorInfo->IsLocallyControlled())
	{
		if (bFromWeaponMesh)
		{
			if (IsValid(TrackedWeaponActor))
			{
				UStaticMeshComponent* WeaponMesh = TrackedWeaponActor->GetCachedMeshComponent();
				if (WeaponMesh && WeaponMesh->DoesSocketExist(SocketName))
				{
					bFound = true;
					return WeaponMesh->GetSocketTransform(SocketName);
				}
			}
		}
		else
		{
			USkeletalMeshComponent* SkelMesh = GetAvatarSkeletalMesh();
			if (SkelMesh && SkelMesh->DoesSocketExist(SocketName))
			{
				bFound = true;
				return SkelMesh->GetSocketTransform(SocketName);
			}
		}
		return FTransform::Identity;
	}

	// Simulated proxy: return the last replicated snapshot
	for (const FARPGRepSocketEntry& Entry : RepSocketData)
	{
		if (Entry.SocketName == SocketName && Entry.bIsWeaponSocket == bFromWeaponMesh)
		{
			bFound = true;
			return Entry.ToTransform();
		}
	}

	return FTransform::Identity;
}

// ─────────────────────────────────────────────────────────────────────────────
// Socket Replication – Internal
// ─────────────────────────────────────────────────────────────────────────────

void UARPGAbilitySystemComponent::Server_UpdateSocketReplicationData()
{
	TArray<FARPGRepSocketEntry> NewSocketData;

	// ── Character skeletal mesh sockets ──────────────────────────────────────
	if (TrackedCharacterSockets.Num() > 0)
	{
		USkeletalMeshComponent* SkelMesh = GetAvatarSkeletalMesh();
		if (SkelMesh)
		{
			for (const FName& SocketName : TrackedCharacterSockets)
			{
				if (SkelMesh->DoesSocketExist(SocketName))
				{
					NewSocketData.Emplace(SocketName, SkelMesh->GetSocketTransform(SocketName), /*bWeapon=*/false);
				}
			}
		}
	}

	// ── Weapon actor static mesh sockets ─────────────────────────────────────
	if (TrackedWeaponSockets.Num() > 0 && IsValid(TrackedWeaponActor))
	{
		UStaticMeshComponent* WeaponMesh = TrackedWeaponActor->GetCachedMeshComponent();
		if (WeaponMesh)
		{
			for (const FName& SocketName : TrackedWeaponSockets)
			{
				if (WeaponMesh->DoesSocketExist(SocketName))
				{
					NewSocketData.Emplace(SocketName, WeaponMesh->GetSocketTransform(SocketName), /*bWeapon=*/true);
				}
			}
		}
	}

	// Only dirty the replicated property (and trigger a net update) if data changed.
	// TArray comparison is O(n) but socket counts are small (typically 1-4 entries).
	if (NewSocketData != RepSocketData)
	{
		RepSocketData = MoveTemp(NewSocketData);

		if (AbilityActorInfo.IsValid() && AbilityActorInfo->AvatarActor.IsValid())
		{
			AbilityActorInfo->AvatarActor->ForceNetUpdate();
		}
	}
}

void UARPGAbilitySystemComponent::OnRep_ReplicatedSocketData()
{
	// Intentionally empty – systems query RepSocketData via GetReplicatedSocketTransform().
	// Bind a delegate here if you need push-style notifications on the client.
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

USkeletalMeshComponent* UARPGAbilitySystemComponent::GetAvatarSkeletalMesh() const
{
	if (!AbilityActorInfo.IsValid() || !AbilityActorInfo->AvatarActor.IsValid())
	{
		return nullptr;
	}

	// ACharacter::GetMesh() is the fastest path for our ARPG characters
	if (ACharacter* Character = Cast<ACharacter>(AbilityActorInfo->AvatarActor.Get()))
	{
		return Character->GetMesh();
	}

	// Fallback for non-Character avatars
	return AbilityActorInfo->AvatarActor->FindComponentByClass<USkeletalMeshComponent>();
}

bool UARPGAbilitySystemComponent::IsMeshOwnedByAvatar(USkeletalMeshComponent* InMesh) const
{
	return IsValid(InMesh)
		&& AbilityActorInfo.IsValid()
		&& AbilityActorInfo->AvatarActor.IsValid()
		 && InMesh->GetOwner() == AbilityActorInfo->AvatarActor.Get();
}
