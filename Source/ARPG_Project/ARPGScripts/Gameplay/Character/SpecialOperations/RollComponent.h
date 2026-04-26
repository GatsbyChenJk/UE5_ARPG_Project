// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "SpecialOperation.h"
#include "Components/ActorComponent.h"
#include "RollComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API URollComponent : public UActorComponent,public ISpecialOperation
{
	GENERATED_BODY()

public:
	URollComponent();
	
	UPROPERTY(Replicated)
	AARPGBaseCharacter* OwnerCharacter;
	
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	FGameplayAbilitySpecHandle RollAbilityHandle;

	UFUNCTION()
	virtual void TriggeredOperation_Implementation() override;

	UFUNCTION(Server, Reliable)
	void Server_SetCharacterRollAbility(const FString& InOperationID);

	UFUNCTION(Server, Reliable,BlueprintCallable)
	void Server_ActivateRollAbility();

	UFUNCTION(BlueprintCallable)
	float GetStaminaCost()
	{
		return StaminaCost;
	};

	UFUNCTION(BlueprintCallable)
	void ResetRollWindow()
	{
		bIsRolling = true;
		CurrentRollWindow = RollWindow;
	}
protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ResetRollState();

	void TickRollWindow(float dt);
private:
	virtual void InitOperation_Implementation(AARPGBaseCharacter* InOwnerCharacter, UAbilitySystemComponent* ASC) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	float StaminaCost = 0.0f;

	UPROPERTY(Replicated)
	bool bIsRolling = false;

	UPROPERTY(Replicated)
	float CurrentRollWindow = 0.0f;

	UPROPERTY(Replicated)
	float RollWindow = 0.0f;
};
