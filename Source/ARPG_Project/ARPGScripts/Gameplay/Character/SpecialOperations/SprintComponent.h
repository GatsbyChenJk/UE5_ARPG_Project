// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "SpecialOperation.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "SprintComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API USprintComponent : public UActorComponent,public ISpecialOperation
{
	GENERATED_BODY()

public:
	USprintComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run Operation")
	float OriginalWalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run Operation")
	float RunSpeedMultiplier = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Run Operation")
	FString OperationID = "Run";
	
	void OnStaminaChanged(const FOnAttributeChangeData& Data);
protected:
	virtual void BeginPlay() override;

	// 角色引用
	UPROPERTY(Replicated)
	AARPGBaseCharacter* OwnerCharacter;

	// ASC引用
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	// 奔跑状态（组件内管理，不再依赖角色类的bIsRunning）
	UPROPERTY(Replicated)
	ECharacterOperationState CurrentState = ECharacterOperationState::Inactive;
	
	FActiveGameplayEffectHandle SprintConsumeEffectHandle;
	FGameplayAbilitySpecHandle SprintAbilityHandle;
public:
	void SetSprintConsumeHandle_Internal(FActiveGameplayEffectHandle InHandle) { SprintConsumeEffectHandle = InHandle; }

	UFUNCTION(BlueprintCallable)
	ECharacterOperationState GetCurrentState() const { return CurrentState; }
	
	UFUNCTION(Server,Reliable)
	void Server_SetCharacterSprintAbility(const FString& InOperationID);
	
// reconstruct start
	UFUNCTION(Server, Reliable)
	void Server_ActivateSprintAbility(bool bIsSprinting);

	UFUNCTION(Server, Reliable)
	void Server_OnSprintCompleted();
// reconstruct end
	UFUNCTION()
	virtual bool StartedOperation_Implementation() override;

	UFUNCTION()
	virtual void CompletedOperation_Implementation(bool bIsInterrupted = false) override;
private:
	virtual void InitOperation_Implementation(AARPGBaseCharacter* InOwnerCharacter, UAbilitySystemComponent* ASC) override;

	void UpdateRunMovementParams(bool bIsRunning);

	virtual FString GetOperationID_Implementation() const override;
	
	virtual ECharacterOperationState GetOperationState_Implementation() const override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
