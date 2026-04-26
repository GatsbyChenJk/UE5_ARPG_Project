// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGScripts/Gameplay/Combat/Execution/Exec_PowerUp.h"


UExec_PowerUp::UExec_PowerUp()
{
	RelevantAttributesToCapture.Add(AttributeStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(AttributeStatics().DefensePowerDef);
	RelevantAttributesToCapture.Add(AttributeStatics().DamageReductionDef);
	RelevantAttributesToCapture.Add(AttributeStatics().HealthDef);
}

void UExec_PowerUp::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// set when onConsume func called
	float AttackPowerUp = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.WeaponAttack.BaseAttackPower")),false,0.0f);
	float DefensePowerUp = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.Defense.Armor")),false,0.0f);
	float DamageReductionPowerUp = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.BasicAttack.Reduction")),false,0.0f);
	float HealingAmount = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("CharacterAttribute.Health.Recover")),false,0.0f);
	
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().AttackPowerProperty,EGameplayModOp::Additive,AttackPowerUp));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().DefensePowerProperty,EGameplayModOp::Additive,DefensePowerUp));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().DamageReductionProperty,EGameplayModOp::Additive,DamageReductionPowerUp));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().HealthProperty,EGameplayModOp::Additive,HealingAmount));
}
