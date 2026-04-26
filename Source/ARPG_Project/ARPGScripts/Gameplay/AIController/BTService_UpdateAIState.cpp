// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_UpdateAIState.h"

#include "ARPGScripts/Gameplay/Character/FAIManifest.h"
#include "InGameAIController.h"

void UBTService_UpdateAIState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AInGameAIController* AIController = Cast<AInGameAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return;

	AIController->SetAIControlState(EAIControlState::Patrol);
	
	AActor* TargetActor = AIController->GetBlackboardTarget();
	if (IsValid(TargetActor))
	{
		AIController->SetAIControlState(EAIControlState::Attack);
	}
}

bool UBTService_UpdateAIState::ShouldAIRoll(AInGameAIController* AIController)
{
	return AIController->GetBlackboardBool("bIsPlayerUsingMagic") ||
		AIController->GetBlackboardBool("bIsHeavyAttack") ||
			AIController->GetBlackboardBool("bIncomingProjectile");
}
