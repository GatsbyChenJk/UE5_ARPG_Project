// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Base.h"
#include "Exec_PowerUp.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UExec_PowerUp : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
	UExec_PowerUp();
	
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
