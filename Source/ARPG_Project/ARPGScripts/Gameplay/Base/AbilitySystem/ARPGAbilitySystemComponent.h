// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ARPGAbilitySystemComponent.generated.h"

class USkeletalMeshComponent;
class AARPGBaseWeapon;

// ─────────────────────────────────────────────────────────────────────────────
// Per-mesh montage structs
// Matches the GASShooter multi-mesh pattern: each skeletal mesh on the avatar
// (e.g. body, weapon attachment point) gets its own montage slot so the server
// can animate them independently and replicate each slot to simulated proxies.
// ─────────────────────────────────────────────────────────────────────────────

/** Local (non-replicated) montage state – one entry per skeletal mesh. */
USTRUCT()
struct ARPG_PROJECT_API FARPGLocalAnimMontageForMesh
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY()
	FGameplayAbilityLocalAnimMontage LocalMontageInfo;

	FARPGLocalAnimMontageForMesh() : Mesh(nullptr) {}
	explicit FARPGLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh) : Mesh(InMesh) {}
};

/** Replicated montage state – drives simulated proxy playback. */
USTRUCT()
struct ARPG_PROJECT_API FARPGRepAnimMontageForMesh
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY()
	FGameplayAbilityRepAnimMontage RepMontageInfo;

	FARPGRepAnimMontageForMesh() : Mesh(nullptr) {}
	explicit FARPGRepAnimMontageForMesh(USkeletalMeshComponent* InMesh) : Mesh(InMesh) {}
};

// ─────────────────────────────────────────────────────────────────────────────
// Socket replication structs
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Replicated world-space transform snapshot for one named socket.
 * Populated on the server each tick and sent to simulated proxies.
 *
 * Used by systems that need authoritative socket positions:
 *   - Hit-detection validation (CharacterAttackComponent::DetectSockets)
 *   - VFX/projectile spawn points on the weapon mesh
 *   - IK target positions on the character mesh
 */
USTRUCT(BlueprintType)
struct ARPG_PROJECT_API FARPGRepSocketEntry
{
	GENERATED_BODY()

public:
	/** Name of the socket on the mesh */
	UPROPERTY(BlueprintReadOnly, Category = "ARPG|Sockets")
	FName SocketName;

	/** World-space location (cm) */
	UPROPERTY(BlueprintReadOnly, Category = "ARPG|Sockets")
	FVector Location = FVector::ZeroVector;

	/** World-space rotation */
	UPROPERTY(BlueprintReadOnly, Category = "ARPG|Sockets")
	FRotator Rotation = FRotator::ZeroRotator;

	/**
	 * True  → socket belongs to the equipped weapon actor's static mesh.
	 * False → socket belongs to the avatar character's skeletal mesh.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ARPG|Sockets")
	bool bIsWeaponSocket = false;

	FARPGRepSocketEntry() : SocketName(NAME_None) {}

	FARPGRepSocketEntry(FName InName, const FTransform& InTransform, bool bWeapon)
		: SocketName(InName)
		, Location(InTransform.GetLocation())
		, Rotation(InTransform.Rotator())
		, bIsWeaponSocket(bWeapon)
	{}

	FTransform ToTransform() const
	{
		return FTransform(Rotation.Quaternion(), Location);
	}

	bool operator==(const FARPGRepSocketEntry& Other) const
	{
		return SocketName == Other.SocketName
			&& Location.Equals(Other.Location)
			&& Rotation.Equals(Other.Rotation)
			&& bIsWeaponSocket == Other.bIsWeaponSocket;
	}

	bool operator!=(const FARPGRepSocketEntry& Other) const
	{
		return !(*this == Other);
	}
};

// ─────────────────────────────────────────────────────────────────────────────
// UARPGAbilitySystemComponent
// ─────────────────────────────────────────────────────────────────────────────

/**
 * Custom Ability System Component for ARPG_Project.
 *
 * Feature 1 – Multi-mesh animation montage replication:
 *   Extends the engine's single-mesh montage replication so the server can
 *   play montages on any skeletal mesh owned by the avatar actor and have
 *   simulated proxies mirror them. Ported from GASShooter by Dan Kestranek.
 *
 * Feature 2 – Socket transform replication:
 *   Named sockets on the character's skeletal mesh and/or the currently
 *   equipped weapon actor's static mesh are sampled every server tick and
 *   replicated to all clients via RepSocketData.
 *
 *   Integration steps:
 *     1. Replace UAbilitySystemComponent with UARPGAbilitySystemComponent in
 *        AAInGamePlayerState.
 *     2. After equipping a weapon, call SetTrackedWeaponActor() and
 *        SetTrackedWeaponSocketNames() from the equipment component (server).
 *     3. Call SetTrackedCharacterSocketNames() with the hit-detection socket
 *        names from FWeaponManifest::DetectSocketNames (server).
 *     4. Query GetReplicatedSocketTransform() anywhere you need an accurate
 *        socket world-space position.
 */
UCLASS()
class ARPG_PROJECT_API UARPGAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UARPGAbilitySystemComponent();

	// ── UAbilitySystemComponent overrides ────────────────────────────────────

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool GetShouldTick() const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	// ── Multi-mesh montage API ────────────────────────────────────────────────

	/**
	 * Play a montage on the specified skeletal mesh with server→client replication.
	 * On the server this also marks the RepAnimMontageInfoForMeshes entry dirty
	 * so simulated proxies receive the update on the next net update.
	 */
	virtual float PlayMontageForMesh(
		UGameplayAbility* AnimatingAbility,
		USkeletalMeshComponent* InMesh,
		FGameplayAbilityActivationInfo ActivationInfo,
		UAnimMontage* Montage,
		float InPlayRate,
		FName StartSectionName = NAME_None,
		bool bReplicateMontage = true);

	/** Play a montage locally on InMesh (called on simulated proxies from OnRep). */
	virtual float PlayMontageSimulatedForMesh(
		USkeletalMeshComponent* InMesh,
		UAnimMontage* Montage,
		float InPlayRate,
		FName StartSectionName = NAME_None);

	virtual void CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.f);
	virtual void StopAllCurrentMontages(float OverrideBlendOutTime = -1.f);
	virtual void StopMontageIfCurrentForMesh(USkeletalMeshComponent* InMesh, const UAnimMontage& Montage, float OverrideBlendOutTime = -1.f);
	virtual void ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability);
	virtual void CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName);
	virtual void CurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName);
	virtual void CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, float InPlayRate);

	bool IsAnimatingAbilityForAnyMesh(UGameplayAbility* Ability) const;
	UGameplayAbility* GetAnimatingAbilityFromAnyMesh();
	TArray<UAnimMontage*> GetCurrentMontages() const;
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);
	int32 GetCurrentMontageSectionIDForMesh(USkeletalMeshComponent* InMesh);
	FName GetCurrentMontageSectionNameForMesh(USkeletalMeshComponent* InMesh);
	float GetCurrentMontageSectionLengthForMesh(USkeletalMeshComponent* InMesh);
	float GetCurrentMontageSectionTimeLeftForMesh(USkeletalMeshComponent* InMesh);

	// ── Socket replication API ────────────────────────────────────────────────

	/**
	 * (Server / owning client) Register socket names to collect each tick from
	 * the avatar character's skeletal mesh.
	 * Typical use: pass FWeaponManifest::DetectSocketNames when a weapon is equipped.
	 * Passing an empty array stops socket replication for the character mesh.
	 */
	UFUNCTION(BlueprintCallable, Category = "ARPG|Sockets")
	void SetTrackedCharacterSocketNames(const TArray<FName>& SocketNames);

	/**
	 * (Server / owning client) Register socket names to collect each tick from
	 * the equipped weapon actor's static mesh.
	 * Passing an empty array stops socket replication for the weapon mesh.
	 */
	UFUNCTION(BlueprintCallable, Category = "ARPG|Sockets")
	void SetTrackedWeaponSocketNames(const TArray<FName>& SocketNames);

	/**
	 * (Server) Set the weapon actor whose mesh sockets should be sampled.
	 * Call from UEquipmentComponent after the weapon actor is spawned and attached.
	 */
	UFUNCTION(BlueprintCallable, Category = "ARPG|Sockets")
	void SetTrackedWeaponActor(AARPGBaseWeapon* InWeapon);

	/** (Server) Clear the tracked weapon (call from UEquipmentComponent on unequip). */
	UFUNCTION(BlueprintCallable, Category = "ARPG|Sockets")
	void ClearTrackedWeaponActor();

	/**
	 * Query the most recent world-space transform for a named socket.
	 *
	 * On the server and autonomous proxy the live mesh is queried directly for
	 * maximum accuracy.  On simulated proxies the last replicated snapshot is
	 * returned.
	 *
	 * @param SocketName      Name of the socket to look up.
	 * @param bFromWeaponMesh True → search weapon-mesh entries, False → character mesh.
	 * @param bFound          Set to true when the socket data was found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ARPG|Sockets")
	FTransform GetReplicatedSocketTransform(FName SocketName, bool bFromWeaponMesh, bool& bFound) const;

protected:
	// ── Montage internal state ────────────────────────────────────────────────

	UPROPERTY()
	TArray<FARPGLocalAnimMontageForMesh> LocalAnimMontageInfoForMeshes;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedAnimMontageForMesh)
	TArray<FARPGRepAnimMontageForMesh> RepAnimMontageInfoForMeshes;

	FARPGLocalAnimMontageForMesh& GetLocalAnimMontageInfoForMesh(USkeletalMeshComponent* InMesh);
	FARPGRepAnimMontageForMesh& GetRepAnimMontageInfoForMesh(USkeletalMeshComponent* InMesh);

	void OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* PredictiveMontage);
	void AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh);
	void AnimMontage_UpdateReplicatedDataForMesh(FARPGRepAnimMontageForMesh& OutRepMontageInfo);

	UFUNCTION()
	virtual void OnRep_ReplicatedAnimMontageForMesh();

	virtual bool IsReadyForReplicatedMontageForMesh();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh,
		UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName, FName NextSectionName);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCurrentMontageJumpToSectionNameForMesh(USkeletalMeshComponent* InMesh,
		UAnimMontage* ClientAnimMontage, FName SectionName);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh,
		UAnimMontage* ClientAnimMontage, float InPlayRate);

	// ── Socket replication internal state ────────────────────────────────────

	/** Socket names to sample from the avatar's skeletal mesh each server tick. */
	UPROPERTY()
	TArray<FName> TrackedCharacterSockets;

	/** Socket names to sample from the weapon actor's static mesh each server tick. */
	UPROPERTY()
	TArray<FName> TrackedWeaponSockets;

	/** Weapon actor reference (server only; set via SetTrackedWeaponActor). */
	UPROPERTY()
	TObjectPtr<AARPGBaseWeapon> TrackedWeaponActor;

	/** Replicated snapshot of socket world-space transforms (server → all clients). */
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedSocketData)
	TArray<FARPGRepSocketEntry> RepSocketData;

	/** Called on the server every tick when sockets are registered; fills RepSocketData. */
	void Server_UpdateSocketReplicationData();

	/** Called on clients when RepSocketData is updated from the server. */
	UFUNCTION()
	void OnRep_ReplicatedSocketData();

	// ── Helpers ───────────────────────────────────────────────────────────────

	/** Returns the avatar actor's primary skeletal mesh component. */
	USkeletalMeshComponent* GetAvatarSkeletalMesh() const;

	/** Returns true if InMesh is valid and owned by the current avatar actor. */
	bool IsMeshOwnedByAvatar(USkeletalMeshComponent* InMesh) const;
};
