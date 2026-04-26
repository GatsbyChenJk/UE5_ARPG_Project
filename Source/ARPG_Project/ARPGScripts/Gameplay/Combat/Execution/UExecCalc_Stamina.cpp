#include "UExecCalc_Stamina.h"

UExecCalc_Stamina::UExecCalc_Stamina()
{
	RelevantAttributesToCapture.Add(AttributeStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(AttributeStatics().DefensePowerDef);
	RelevantAttributesToCapture.Add(AttributeStatics().DamageReductionDef);
	RelevantAttributesToCapture.Add(AttributeStatics().HealthDef);
	RelevantAttributesToCapture.Add(AttributeStatics().StaminaDef);
}

void UExecCalc_Stamina::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
											   FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// get from current special operation
	float WeaponStaminaDrain = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Combat.WeaponAttack.WeaponStaminaCost")),false,0.0f);
	float SpecialOperationDrain = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("SpecialOperation.Activate")),false,0.0f);
	// TODO:Add Armor Drain Data
	
	float TotalCost = WeaponStaminaDrain + SpecialOperationDrain;
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(AttributeStatics().StaminaProperty,EGameplayModOp::Additive,-TotalCost));
}