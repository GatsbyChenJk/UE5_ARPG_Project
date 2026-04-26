// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterDamageComponent.h"

#include "AInGameCharacter.h"
#include "InGameAICharacter.h"
#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Combat/AInGamePlayerState.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterAttackComponent.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterShieldComponent.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/ARPGBaseWeapon.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/CharacterMeleeWeapon.h"
#include "Engine/OverlapResult.h"
#include "Net/UnrealNetwork.h"

UCharacterDamageComponent::UCharacterDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCharacterDamageComponent::Server_SetCharacterGetDamage_Implementation(FCharacterData CharData)
{
	AAInGameCharacter* PlayerChar = Cast<AAInGameCharacter>(GetOwner());
	AAInGamePlayerState* PlayerState = PlayerChar ? Cast<AAInGamePlayerState>(PlayerChar->GetPlayerState()) : nullptr;
	if (auto PlayerASC = PlayerState->GetAbilitySystemComponent())
	{
		auto HurtAbility = CharData.GetCharacterManifest()->CharacterHurtAbility;
		if (IsValid(HurtAbility))
		{
			FGameplayAbilitySpec HurtAbilitySpec(HurtAbility);
			GetDamageAbility = PlayerASC->GiveAbility(HurtAbilitySpec);
		}
	}
}

void UCharacterDamageComponent::Server_SetAIGetDamage_Implementation(FAIManifest AIConfigData)
{
	AInGameAICharacter* AIChar = Cast<AInGameAICharacter>(GetOwner());
	if (auto AIASC = AIChar->GetAbilitySystemComponent())
	{
		if (IsValid(AIConfigData.AIHurtAbility))
		{
			FGameplayAbilitySpec HurtAbilitySpec(AIConfigData.AIHurtAbility);
			GetDamageAbility = AIASC->GiveAbility(HurtAbilitySpec);
		}
	}
}

void UCharacterDamageComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCharacterDamageComponent,SocketName);
}

void UCharacterDamageComponent::SendDamageEvent(AActor* HitActor, FGameplayTag DamageTag, FGameplayEventData EventData)
{
	if (!IsValid(HitActor)) return;
	
	if (AAInGameCharacter* PlayerChar = Cast<AAInGameCharacter>(HitActor))
	{
		APlayerState* PlayerState = PlayerChar->GetPlayerState();
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(PlayerState,DamageTag,EventData);
	}
	else
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitActor,DamageTag,EventData);
	}
}

void UCharacterDamageComponent::Server_SendApplyDamageEventToActor_Implementation(AActor* HitActor,FGameplayTag DamageTag,FGameplayEventData EventData)
{
	// GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Red,FString::Printf(TEXT("HitActor Name:%s,AttackActor:%s"),
	// 	*HitActor->GetName(),*AttackCharacter->GetName()));
	SendDamageEvent(HitActor, DamageTag, EventData);
}

// special use for AI
void UCharacterDamageComponent::NetMulticast_ApplyDamageToHitActors_Implementation(const FGameplayTag& DamageTag)
{
	for (auto HitActor: StoredHitActors)
	{
		// ai doesnt have owning channel to send server rpc
		// so we need to find actor with owning role
		if (AARPGBaseCharacter* HitChar = Cast<AARPGBaseCharacter>(HitActor))
		{
			if (const auto DamageComp = HitChar->GetCharacterDamageComponent())
			{
			 	FGameplayEventData Event;
				Event.EventMagnitude = AttackCharacter->GetAttributeSet()->GetAttackPower();
				
				DamageComp->Server_SendApplyDamageEventToActor(HitActor,DamageTag,Event);
			}
		}
	}
	
	StoredHitActors.Empty();
}


void UCharacterDamageComponent::ApplyDamageToHitActors(const FGameplayTag& DamageTag)
{
	//GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Red,FString::Printf(TEXT("Apply to actor Start,Current Role:%d"),AttackCharacter->GetLocalRole()));
	for (auto HitActor: StoredHitActors)
	{
		FGameplayEventData EventData;
		EventData.EventMagnitude = AttackCharacter->GetAttributeSet()->GetAttackPower();
		
		Server_SendApplyDamageEventToActor(HitActor,DamageTag,EventData);
	}
	
	StoredHitActors.Empty();
}

void UCharacterDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	AttackCharacter = Cast<AARPGBaseCharacter>(GetOwner());
}

void UCharacterDamageComponent::TickDetection_UnarmedAI(FVector HitLocation)
{
	if (IsValid(AttackCharacter))
	{
		//GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::Red,FString::Printf(TEXT("TickDetection_AI CurrentRole:%d"),AttackCharacter->GetLocalRole()));
		if (!AttackCharacter->HasAuthority())
		{
			SphereOverlapDetection(HitLocation,SphereDetectRadius);
		}
	}
}

void UCharacterDamageComponent::TickDetection_EquippedAI(FName DetectSocket)
{
	if (IsValid(AttackCharacter))
	{
		if (AttackCharacter->HasAuthority())
		{
			return;
		}
		
		// GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Red,
		// FString::Printf(TEXT("Current Socket: %s,Attack Role:%d"),*SocketName.ToString(),AttackCharacter->GetLocalRole()));

		FVector DetectLocation = GetOwner()->GetActorLocation();
		FQuat DetectRotation = GetOwner()->GetActorRotation().Quaternion();
		
		if (!IsValid(CurrentMeleeWeapon))
		{
			if (auto AttackComp = AttackCharacter->GetCharacterAttackComponent())
			{
				CurrentMeleeWeapon = AttackComp->GetCurrentEquippedWeapon();
			}
		}

		if (!IsValid(CurrentMeleeWeapon)) return;
		
		if (const auto MeshComp = CurrentMeleeWeapon->GetCachedMeshComponent())
		{
			FTransform WeaponTranfrom = MeshComp->GetSocketTransform(DetectSocket);
			DetectLocation = WeaponTranfrom.GetLocation();
			DetectRotation = WeaponTranfrom.GetRotation();
		}

		CapsuleOverlapDetection(DetectLocation,CapsuleDetectRadius,CapsuleHalfHeight,DetectRotation);
	}
}

void UCharacterDamageComponent::TickDetection_UnarmedCharacter()
{
	if (IsValid(AttackCharacter))
	{
		if (AttackCharacter->HasAuthority())
		{
			return;
		}
		
		 // GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Red,
			// FString::Printf(TEXT("Current Socket: %s,Attack Role:%d"),*SocketName.ToString(),AttackCharacter->GetLocalRole()));

		FVector HitLocation = GetOwner()->GetActorLocation();
		 if (const auto CharMesh = AttackCharacter->GetMesh())
		 {
			auto SocketTransform = CharMesh->GetSocketTransform(FName(*SocketName));
			HitLocation = SocketTransform.GetLocation();
			//UE_LOG(LogTemp,Warning,TEXT("HitLocation:%s"),*HitLocation.ToString());
		 }
		
		Server_SphereOverlapDetection(HitLocation,SphereDetectRadius);
		
	}
}

void UCharacterDamageComponent::TickDetection_EquippedCharacter()
{
	if (IsValid(AttackCharacter))
	{
		if (AttackCharacter->HasAuthority())
		{
			return;
		}
		
		// GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Red,
		// FString::Printf(TEXT("Current Socket: %s,Attack Role:%d"),*SocketName.ToString(),AttackCharacter->GetLocalRole()));

		FVector DetectLocation = GetOwner()->GetActorLocation();
		FQuat DetectRotation = GetOwner()->GetActorRotation().Quaternion();
		
		if (!IsValid(CurrentMeleeWeapon))
		{
			if (auto AttackComp = AttackCharacter->GetCharacterAttackComponent())
			{
				CurrentMeleeWeapon = AttackComp->GetCurrentEquippedWeapon();
			}
		}

		if (!IsValid(CurrentMeleeWeapon)) return;
		
		if (const auto MeshComp = CurrentMeleeWeapon->GetCachedMeshComponent())
		{
			FTransform WeaponTranfrom = MeshComp->GetSocketTransform(FName(*SocketName));
			DetectLocation = WeaponTranfrom.GetLocation();
			DetectRotation = WeaponTranfrom.GetRotation();
		}

		Server_CapsuleOverlapDetection(DetectLocation,CapsuleDetectRadius,CapsuleHalfHeight,DetectRotation);
	}
}

void UCharacterDamageComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCharacterDamageComponent::Server_SphereOverlapDetection_Implementation(const FVector& Origin, float Radius)
{
	SphereOverlapDetection(Origin,Radius);
}

void UCharacterDamageComponent::SphereOverlapDetection(const FVector& Origin, float Radius)
{
	if (!GetWorld())
	{
		return;
	}
	
	FCollisionQueryParams QueryParams = InitCollisionParams();
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(Radius);
	
	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		Origin,
		FQuat::Identity,
		ECC_GameTraceChannel3, // Melee
		CollisionShape,
		QueryParams
	);
	
	UpdateHitResult(OverlapResults);

#if WITH_EDITOR
	if (bDrawDebugSpheres)
	{
		DrawDebugSphere(
			GetWorld(),
			Origin,
			Radius,
			12, // 细分段数
			FColor::Red,
			false, // 是否持续
			2.0f,  // 持续时间
			0,     // 深度优先级
			1.0f   // 线粗
		);
	}
#endif
	
}

void UCharacterDamageComponent::Server_CapsuleOverlapDetection_Implementation(const FVector& Origin, float Radius,
	float HalfHeight,const FQuat& SocketQuat)
{
	CapsuleOverlapDetection(Origin,Radius,HalfHeight,SocketQuat);
}

void UCharacterDamageComponent::CapsuleOverlapDetection(const FVector& Origin, float Radius, float HalfHeight,
	const FQuat& SocketQuat)
{
	if (!GetWorld())
	{
		return;
	}
	
	FCollisionQueryParams QueryParams = InitCollisionParams();
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(Radius,HalfHeight);
	
	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		Origin,
		SocketQuat,  
		ECC_GameTraceChannel3, // Melee
		CollisionShape,
		QueryParams
	);
	
	UpdateHitResult(OverlapResults);

#if WITH_EDITOR
	if (bDrawDebugCapsule)
	{
		DrawDebugCapsule(
			GetWorld(),
			Origin,
			Radius,
			HalfHeight,
			FQuat::Identity,
			FColor::Red,
			false,
			2.0f,
			0,
			1.0f);
	}
#endif
}

void UCharacterDamageComponent::CapsuleOverlapDetection(const FVector& Origin, float Radius, float HalfHeight,const FRotator& SocketRot)
{
	FQuat Rotation = SocketRot.Quaternion();
	CapsuleOverlapDetection(Origin,Radius,HalfHeight,Rotation);
}

FCollisionQueryParams UCharacterDamageComponent::InitCollisionParams()
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false; 
	QueryParams.bReturnPhysicalMaterial = false;
	
	if (IsValid(GetOwner()))
	{
		QueryParams.AddIgnoredActor(GetOwner());

		// TODO : 多人情况下排除队友
	}

	return QueryParams;
}

void UCharacterDamageComponent::UpdateHitResult(TArray<FOverlapResult>& OverlapResults)
{
	for (const FOverlapResult& Result : OverlapResults)
	{
		if (AActor* HitActor = Result.GetActor())
		{
			if (!StoredHitActors.Contains(HitActor))
			{
				GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Red,
					FString::Printf(TEXT("HitActor: %s"),*HitActor->GetName()));
				StoredHitActors.Add(HitActor);
			}
		}
	}
}

