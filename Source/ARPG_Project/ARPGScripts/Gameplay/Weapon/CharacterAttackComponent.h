// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "EquipmentComponent.h"
#include "Components/ActorComponent.h"
#include "CharacterAttackComponent.generated.h"


class UGameplayAbility;
struct FGameplayAbilitySpecHandle;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UCharacterAttackComponent : public UEquipmentComponent
{
	GENERATED_BODY()

public:
	UCharacterAttackComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void FindAndAddEquipmentAbilities(const FString& WeaponID, UAbilitySystemComponent* ASC) override;

	UFUNCTION(Server,Reliable,Blueprintable)
	void Server_CharacterHandleAttackSpecial(EEquipmentActivateType ActivateType);
	
	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_CharacterHandleAttack();

	UFUNCTION(BlueprintCallable)
	void SetIsActtackCombo(bool IsCombo){ bIsAttackCombo = IsCombo; }

	UFUNCTION(BlueprintCallable)
	void ResetCurrentWindow() { CurrentWindow = AttackWindow;};
	
	UFUNCTION(BlueprintCallable,Server,Reliable)
	void Server_GetDetectSocketNameByCurrentCombo();

	// UFUNCTION(client,Reliable)
	// void Client_SetDetectSocketName(const FName& SocketName);

	UFUNCTION(BlueprintCallable)
	FName GetDetectSocketName(int32 currentCombo) const { return DetectSockets.IsValidIndex(currentCombo) ? DetectSockets[currentCombo]: FName(); };

	virtual UAnimMontage* GetCurrentMontage() override
	{
		if (GetOwnerRole() == ROLE_AutonomousProxy) CurrentAttackCombo = (CurrentAttackCombo+1) % EquipmentSpecHandles.Num();
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Current AttackMontage Index:%d,Current Role:%d"),CurrentAttackCombo,GetOwnerRole()));
		return AttackMontages.IsValidIndex(CurrentAttackCombo) ? AttackMontages[CurrentAttackCombo] : nullptr;
	};
protected:

	virtual void ActivateSpecialAbilities(UAbilitySystemComponent* ASC, EEquipmentActivateType ActivateType) override;

	void ActivateAttack_Internal(UAbilitySystemComponent* ASC);

	virtual void BeginPlay() override;
	void ResetComboState();
	void TickCurrentWindow(float DeltaTime);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	UPROPERTY(Replicated)
	TArray<FName> DetectSockets;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	UPROPERTY(Replicated)
	float CurrentWindow;

	UPROPERTY(Replicated)
	float AttackWindow{0.f};

	UPROPERTY(Replicated)
	bool bIsAttackCombo{false};

	UPROPERTY(Replicated)
	int32 CurrentAttackCombo{0};

	int32 PendingSocketIndex{INDEX_NONE};

	UFUNCTION(Server,Reliable)
	void Server_CharacterActivateAbility();
};
