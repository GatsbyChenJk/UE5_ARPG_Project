// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGScripts/Gameplay/Combat/Execution/Exec_StaminaCost.h"


UExec_StaminaCost::UExec_StaminaCost()
{
	RelevantAttributesToCapture.Add(AttributeStatics().AttackStaminaCostDef);
	RelevantAttributesToCapture.Add(AttributeStatics().DefenseStaminaCostDef);
}

void UExec_StaminaCost::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// set when onEquip func called
	float WeaponAttackCost = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.WeaponAttack.WeaponStaminaCost")),false,0.0f);
	// TODO:create new tag
	float WeaponDefenseCost = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("CharacterAttribute.Stamina.Consume")),false,0.0f);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().AttackStaminaCostProperty,EGameplayModOp::Additive,WeaponAttackCost));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().DefenseStaminaCostProperty,EGameplayModOp::Additive,WeaponDefenseCost));
}
