// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGScripts/Gameplay/Combat/Execution/Exec_Money.h"


UExec_Money::UExec_Money()
{
	RelevantAttributesToCapture.Add(AttributeStatics().CharacterMoneyDef);
}

void UExec_Money::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// set when OnSell func called
	float ItemValue = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("CharacterAttribute.Money.Gain")),false,0.0f);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().CharacterMoneyProperty,EGameplayModOp::Additive,ItemValue));
}
