#pragma once
#include "ExecCalc_Base.h"
#include "UExecCalc_Stamina.generated.h"

UCLASS()
class ARPG_PROJECT_API UExecCalc_Stamina : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UExecCalc_Stamina();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
