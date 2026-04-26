// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_UpdateAIState.generated.h"

class AInGameAIController;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UBTService_UpdateAIState : public UBTService_BlueprintBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "AI Behavior")
	float StartDodgeStaminaPercent = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "AI Behavior")
	float HealingHealthPercent = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
	float StartDefenseDistance = 10.0f;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	bool ShouldAIRoll(AInGameAIController* AIController);
};
