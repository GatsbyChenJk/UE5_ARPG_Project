// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Character/FAIManifest.h"
#include "ARPGScripts/Gameplay/Character/FCharacterManifest.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilities/Public/AbilitySystemBlueprintLibrary.h"
#include "CharacterDamageComponent.generated.h"


class AARPGBaseWeapon;
class ACharacterMeleeWeapon;
class AARPGBaseCharacter;
class AAInGameCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UCharacterDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterDamageComponent();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetCharacterGetDamage(FCharacterData CharData);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetAIGetDamage(FAIManifest AIConfigData);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void SendDamageEvent(AActor* HitActor, FGameplayTag DamageTag, FGameplayEventData EventData);

	UFUNCTION(Server,Reliable)
	void Server_SendApplyDamageEventToActor(AActor* HitActor,FGameplayTag DamageTag,FGameplayEventData EventData);
	
	//TODO:Fix wrong hurt logic
	UFUNCTION(BlueprintCallable,NetMulticast,Reliable)
	void NetMulticast_ApplyDamageToHitActors(const FGameplayTag& DamageTag);
	
	UFUNCTION(blueprintCallable)
	void ApplyDamageToHitActors(const FGameplayTag& DamageTag);
	
	UFUNCTION(BlueprintCallable)
	void SetIsDamageDetect(bool bIsInDetect) { bIsDetecting = bIsInDetect; };

	UFUNCTION(BlueprintCallable)
	void SetDetectSocketName(const FName& DetectSocket) {SocketName = DetectSocket.ToString(); };

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void TickDetection_UnarmedAI(FVector HitLocation);

	UFUNCTION(BlueprintCallable)
	void TickDetection_EquippedAI(FName DetectSocket);

	UFUNCTION(BlueprintCallable)
	void TickDetection_UnarmedCharacter();

	UFUNCTION(BlueprintCallable)
	void TickDetection_EquippedCharacter();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// call when character weapon changed
	// case 1 : when character init
	// case 2 : when character equip other weapon
	// UFUNCTION(BlueprintCallable)
	// void OnCharacterWeaponSwitched();
	//
	// UFUNCTION(BlueprintCallable)
	// ACharacterMeleeWeapon* GetCurrentMeleeWeapon();
protected:
	UFUNCTION(Server,Reliable)
	void Server_SphereOverlapDetection(const FVector& Origin, float Radius);
	
	void SphereOverlapDetection(const FVector& Origin, float Radius);

	UFUNCTION(Server,Reliable)
	void Server_CapsuleOverlapDetection(const FVector& Origin, float Radius, float HalfHeight,const FQuat& SocketQuat);

	void CapsuleOverlapDetection(const FVector& Origin, float Radius, float HalfHeight,const FQuat& SocketQuat);

	void CapsuleOverlapDetection(const FVector& Origin, float Radius, float HalfHeight,const FRotator& SocketRot);
	
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Debug Collision | Sphere")
	bool bDrawDebugSpheres{false};

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Debug Collision| Sphere")
	float SphereDetectRadius = 5.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Debug Collision | Capsule")
	bool bDrawDebugCapsule{false};

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Debug Collision| Capsule")
	float CapsuleDetectRadius = 5.f;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Debug Collision | Capsule")
	float CapsuleHalfHeight = 5.f;
	
	UPROPERTY(Replicated,EditAnywhere,BlueprintReadWrite, Category = "Debug Collision")
	FString SocketName;

private:
	FCollisionQueryParams InitCollisionParams();
	
	void UpdateHitResult(TArray<FOverlapResult>& OverlapResults);

	UPROPERTY()
	TArray<AActor*> StoredHitActors;

	bool bIsDetecting{false};

	UPROPERTY()
	TObjectPtr<AARPGBaseCharacter> AttackCharacter;

	UPROPERTY()
	TObjectPtr<AARPGBaseWeapon> CurrentMeleeWeapon;
	
	//TODO: add damage calculation logic in GE
	FGameplayAbilitySpecHandle GetDamageAbility;
};
