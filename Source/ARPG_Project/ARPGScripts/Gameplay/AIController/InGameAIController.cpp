// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameAIController.h"

#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Base/GameStates/CharacterManager.h"
#include "ARPGScripts/Gameplay/Character/FAIManifest.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameAICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"


AInGameAIController::AInGameAIController()
{
	// PrimaryActorTick.bCanEverTick = true;
	// Note: We use the base AAIController's BlackboardComp (accessed via GetBlackboardComponent())
	// instead of creating our own to ensure RunBehaviorTree works correctly
}

void AInGameAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AInGameAIController::InitBasicAIConfig(FAIManifest AIConfigData)
{
	SetBlackboard(AIConfigData);
	SetBehaviorTree(AIConfigData);
}

void AInGameAIController::SetBlackboard(FAIManifest AIConfigData)
{
	if (AIConfigData.AIBlackboard)
	{
		BlackboardData = AIConfigData.AIBlackboard;
	}
}

void AInGameAIController::SetBehaviorTree(FAIManifest AIConfigData)
{
	if (AIConfigData.AITree)
	{
		AIBehaviorTree = AIConfigData.AITree;
	}
}

void AInGameAIController::SetAIControlState(EAIControlState AIControlState)
{
	UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (IsValid(BBComp))
	{
		BBComp->SetValueAsEnum(FName("ControlState"),(uint8)AIControlState);
	}
}

void AInGameAIController::SetPatrolPath(AActor* Path)
{
	PatrolPath = Path;
}

void AInGameAIController::SetBlackboardTarget(AActor* TargetActor)
{
	UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (IsValid(BBComp) && IsValid(TargetActor))
	{
		AAInGameCharacter* PlayerChar = Cast<AAInGameCharacter>(TargetActor);
		BBComp->SetValueAsObject(FName("TargetActor"), PlayerChar);
	}
}

AActor* AInGameAIController::GetBlackboardTarget() const
{
	const UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (IsValid(BBComp))
	{
		if (AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(FName("TargetActor"))))
		{
			return TargetActor;
		}
	}
	return nullptr;
}

void AInGameAIController::SetBlackboardBool(FName VarName, bool bCondition)
{
	UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (IsValid(BBComp))
	{
		BBComp->SetValueAsBool(VarName, bCondition);
	}
}

bool AInGameAIController::GetBlackboardBool(FName VarName) const
{
	const UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (IsValid(BBComp))
	{
		return BBComp->GetValueAsBool(VarName);
	}
	return false;
}

void AInGameAIController::SetBlackboardFloat(FName VarName, float InFloat)
{
	UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (IsValid(BBComp))
	{
		BBComp->SetValueAsFloat(VarName, InFloat);
	}
}

float AInGameAIController::GetBlackboardFloat(FName VarName) const
{
	const UBlackboardComponent* BBComp = GetBlackboardComponent();
	if (IsValid(BBComp))
	{
		return BBComp->GetValueAsFloat(VarName);
	}
	return 0.0f;
}

const UBlackboardComponent* AInGameAIController::GetCachedBlackboardComponent() const
{
	return GetBlackboardComponent();
}

void AInGameAIController::StartBehaviorTree()
{
	if (!AIBehaviorTree)
	{
		return;
	}

	UBlackboardComponent* BBComp = GetBlackboardComponent();

	// Ensure BlackboardComponent exists - AAIController doesn't auto-create it
	if (!BBComp)
	{
		BBComp = NewObject<UBlackboardComponent>(this, TEXT("BlackboardComp"));
		BBComp->RegisterComponent();
		Blackboard = BBComp;
	}

	// Initialize blackboard with the blackboard asset from config or behavior tree
	if (BlackboardData)
	{
		BBComp->InitializeBlackboard(*BlackboardData);
	}

	RunBehaviorTree(AIBehaviorTree);
}

void AInGameAIController::SyncTeamFromPawn()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		// 尝试从基类获取阵营
		const AARPGBaseCharacter* BaseCharacter = Cast<AARPGBaseCharacter>(ControlledPawn);
		if (BaseCharacter)
		{
			CurrentTeamID = BaseCharacter->GetTeamID();
		}
		else
		{
			// 默认敌人阵营
			CurrentTeamID = 1;
		}
	}
}

void AInGameAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 同步阵营设置
	SyncTeamFromPawn();

	// Start behavior tree first to initialize blackboard, then set initial state
	StartBehaviorTree();

	// Now that blackboard is initialized, set the initial AI state
	SetAIControlState(EAIControlState::Patrol);
}



