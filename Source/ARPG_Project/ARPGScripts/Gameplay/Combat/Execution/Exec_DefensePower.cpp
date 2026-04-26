// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGScripts/Gameplay/Combat/Execution/Exec_DefensePower.h"


UExec_DefensePower::UExec_DefensePower()
{
	RelevantAttributesToCapture.Add(AttributeStatics().DefensePowerDef);
}

void UExec_DefensePower::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	//const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// set when onEquip func called
	float WeaponDefensePower = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.Defense.Armor")),false,0.0f);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().DefensePowerProperty
		,EGameplayModOp::Additive,WeaponDefensePower));
}
