// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ARPG_ProjectCharacter.h"
#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/SprintComponent.h"
#include "GameFramework/Character.h"
#include "TargetLockSystem/PlayerTargetLockComp.h"
#include "AInGameCharacter.generated.h"

class UARPGAbilitySystemComponent;
class UARPGEventData_Weapon;
class USprintComponent;
class UARPGEventData;
class UCharacterAttackComponent;
class UCharacterDamageComponent;
class AAInGamePlayerState;
class UInGameCharacterAttributeSet;

UCLASS()
class ARPG_PROJECT_API AAInGameCharacter : public AARPGBaseCharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAInGameCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaSeconds) override;

	// GAS start

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 返回类型安全的 ARPG ASC（从 PlayerState 获取），供装备/蒙太奇系统使用 */
	virtual UARPGAbilitySystemComponent* GetARPGAbilitySystemComponent() const override;

	virtual const UInGameCharacterAttributeSet* GetAttributeSet() const override;

	virtual void OnRep_PlayerState() override;

	UFUNCTION()
	void ARPG_CharacterEquipWeapon(UARPGEventData_Weapon* EventData);
	
	UFUNCTION()
	void ARPG_CharacterUnequipWeapon(UARPGEventData_Weapon* EventData);

	// GAS end

	// character input extent start
	
	// sprint start
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Character Controll")
	TObjectPtr<UInputAction> RunningAction;

	UFUNCTION()
	void OnStopRunning();

	UFUNCTION()
	void OnStartRunning();
	
	virtual void SetSprintConsumeHandle(FActiveGameplayEffectHandle InHandle) override
	{
		if (IsValid(CachedSprintComp))
		CachedSprintComp->SetSprintConsumeHandle_Internal(InHandle);
	}
	
	// virtual void SetSprintRecoverHandle(FActiveGameplayEffectHandle InHandle) override
	// {
	// 	if (IsValid(CachedSprintComp))
	// 	CachedSprintComp->SetSprintRecoverHandle_Internal(InHandle);
	// }
	// sprint end

	// dodge start
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Abilities")
	TObjectPtr<UInputAction> RollAction;
	
	UFUNCTION()
	void OnRolling();
	// dodge end

	// character input extent end
	
	// character combat start
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Abilities")
	TObjectPtr<UInputAction> AttackAction;

	UFUNCTION()
	void OnCharacterAttack();
	
	// shield start
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Abilities")
	TObjectPtr<UInputAction> DenfenseAction;
	
	UFUNCTION()
	void OnCharacterDefenseStart();

	UFUNCTION()
	void OnCharacterDefenseEnd();
	// shield end

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Combat")
	TObjectPtr<UInputAction> DeadAction;
	
	UFUNCTION(Server,Reliable)
	void OnCharacterDead();

	virtual void CharacterDeath() override;
	// character combat end

	// Team Affiliation start
public:
	/** 获取玩家阵营ID（重写基类，返回玩家阵营0） */
	virtual int32 GetTeamID() const override { return 0; }
	// Team Affiliation end

	// getter start
	UFUNCTION(blueprintCallable)
	UPlayerTargetLockComp* GetPlayerTargetLockComp();
	// getter end
protected:
	UFUNCTION()
	void TryInitializeClientASC();

	void SetupAndInitializeASC(AAInGamePlayerState* PlayerState, UARPGAbilitySystemComponent* ASC);
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Player Components", meta = (AllowPrivateAccess = "true"))
	UPlayerTargetLockComp* CachedPlayerTargetLockComp;
};
