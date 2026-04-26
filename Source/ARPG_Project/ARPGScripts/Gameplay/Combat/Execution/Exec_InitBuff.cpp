// Fill out your copyright notice in the Description page of Project Settings.


#include "Exec_InitBuff.h"

UExec_InitBuff::UExec_InitBuff()
{
	RelevantAttributesToCapture.Add(AttributeStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(AttributeStatics().DefensePowerDef);
	RelevantAttributesToCapture.Add(AttributeStatics().HealthDef);
	RelevantAttributesToCapture.Add(AttributeStatics().MaxHealthDef);
}

void UExec_InitBuff::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	float BuffAttackPower = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.WeaponAttack.BaseAttackPower")),false,0.0f);
	float BuffDefensePower = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.Defense.Armor")),false,0.0f);
	float BuffHealthPowerUp = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("CharacterAttribute.Health.Recover")),false,0.0f);
	
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().AttackPowerProperty,EGameplayModOp::Additive,BuffAttackPower));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().DefensePowerProperty,EGameplayModOp::Additive,BuffDefensePower));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().HealthProperty,EGameplayModOp::Additive,BuffHealthPowerUp));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().MaxHealthProperty,EGameplayModOp::Additive,BuffHealthPowerUp));

}
