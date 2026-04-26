#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"
#include "ExecCalc_Base.generated.h"

USTRUCT()

struct FAttributeStatics
{
	GENERATED_BODY()
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageReduction);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Stamina);
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackStaminaCost);
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefenseStaminaCost);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CharacterMoney);

	DECLARE_ATTRIBUTE_CAPTUREDEF(MaxHealth);

	FAttributeStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, AttackPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, DefensePower, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, DamageReduction, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, Health, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, Stamina, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, AttackStaminaCost,Source,true)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, DefenseStaminaCost,Source,true)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, CharacterMoney, Source, true);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UInGameCharacterAttributeSet, MaxHealth, Source, true);
	}
};

static const FAttributeStatics& AttributeStatics()
{
	static FAttributeStatics Statics;
	return Statics;
}