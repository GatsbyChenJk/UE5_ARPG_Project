// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGScripts/Gameplay/Combat/Execution/UExec_DamageCalculation.h"


UUExec_DamageCalculation::UUExec_DamageCalculation()
{
	RelevantAttributesToCapture.Add(AttributeStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(AttributeStatics().DefensePowerDef);
	RelevantAttributesToCapture.Add(AttributeStatics().DamageReductionDef);
	RelevantAttributesToCapture.Add(AttributeStatics().HealthDef);
	RelevantAttributesToCapture.Add(AttributeStatics().StaminaDef);
}

void UUExec_DamageCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                      FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// get from hurt GA by weapon class
	float TotalAttackPower = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.WeaponAttack.BaseAttackPower")),false,0.0f);
	float EquipArmorDefense = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.Defense.Armor")),false,0.0f);

	// get from owner attribute
	float SourceDefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(AttributeStatics().DefensePowerDef, FAggregatorEvaluateParameters(), SourceDefensePower);
	
	float TotalDefensePower = EquipArmorDefense + SourceDefensePower;
	float FinalDamage = TotalAttackPower - TotalDefensePower;
	FinalDamage = FMath::Max(0.0f, FinalDamage);

	// when owner is defensing,cost stamina
	float DefenseStaminaDrain = 0.0f;
	if (IsValid(SourceASC))
	{
		if (SourceASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Combat.Defense.Activate"))))
		{
			DefenseStaminaDrain = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("CharacterAttribute.Stamina.Consume")),false,0.0);
			// TODO : add shield defense power
		}
	}

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().HealthProperty,EGameplayModOp::Additive,-FinalDamage));
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().StaminaProperty,EGameplayModOp::Additive,-DefenseStaminaDrain));
}
