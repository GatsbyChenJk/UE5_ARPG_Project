// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "ARPGScripts/Gameplay/Character/FAIManifest.h"
#include "InGameAIController.generated.h"

struct FAIManifest;

UCLASS()
class ARPG_PROJECT_API AInGameAIController : public AAIController
{
	GENERATED_BODY()

public:
	AInGameAIController();

	virtual void BeginPlay() override;
	// basic ai config helper func
	UFUNCTION(BlueprintCallable, Category = "AI")
	void InitBasicAIConfig(FAIManifest AIConfigData);
	
	void SetBlackboard(FAIManifest AIConfigData);
	
	void SetBehaviorTree(FAIManifest AIConfigData);

	UFUNCTION(BlueprintCallable)
	void SetAIControlState(EAIControlState AIControlState);

	void SetPatrolPath(AActor* Path);
	
	UFUNCTION(BlueprintCallable)
	AActor* GetPatrolPath() {return PatrolPath;};

	// blackboard helper
	UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
	void SetBlackboardTarget(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "AI|Blackboard")
	AActor* GetBlackboardTarget() const;
	
	UFUNCTION()
	void SetBlackboardBool(FName VarName,bool bCondition);

	UFUNCTION()
	bool GetBlackboardBool(FName VarName) const;

	UFUNCTION()
	void SetBlackboardFloat(FName VarName,float InFloat);

	UFUNCTION()
	float GetBlackboardFloat(FName VarName) const;
	const UBlackboardComponent* GetCachedBlackboardComponent() const;
	//
	UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
	void StartBehaviorTree();

	// Team Affiliation start
public:
	/** 从控制的Pawn同步阵营设置 */
	void SyncTeamFromPawn();

	/** IGenericTeamAgentInterface - 获取团队ID */
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(CurrentTeamID); }

	/** IGenericTeamAgentInterface - 设置团队ID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override { CurrentTeamID = NewTeamID.GetId(); }

protected:
	virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY()
	UBlackboardData* BlackboardData;
	UPROPERTY()
	UBehaviorTree* AIBehaviorTree;
private:
	// Note: We use the base AAIController's BlackboardComp instead of creating our own
	// This ensures RunBehaviorTree works correctly with the correct blackboard

	UPROPERTY()
	AActor* PatrolPath;

	// Team Affiliation end
	UPROPERTY()
	uint8 CurrentTeamID = 1; // 默认敌人阵营
};
