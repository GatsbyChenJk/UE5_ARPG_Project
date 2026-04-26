// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Base.h"
#include "UExec_DamageCalculation.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UUExec_DamageCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
	UUExec_DamageCalculation();
	
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
