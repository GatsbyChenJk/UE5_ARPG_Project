// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGScripts/Gameplay/Combat/Execution/Exec_AttackDamage.h"


UExec_AttackDamage::UExec_AttackDamage()
{
	RelevantAttributesToCapture.Add(AttributeStatics().AttackPowerDef);
}

void UExec_AttackDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	//const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// set when onEquip func called
	float WeaponAttackPower = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.WeaponAttack.BaseAttackPower")),false,0.0f);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().AttackPowerProperty,EGameplayModOp::Additive,WeaponAttackPower));
}
