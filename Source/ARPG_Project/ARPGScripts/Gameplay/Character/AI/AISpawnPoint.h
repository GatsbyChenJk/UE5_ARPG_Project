// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Character/FAIManifest.h"
#include "Engine/TargetPoint.h"
#include "AISpawnPoint.generated.h"


class AInGameAIController;
class AInGameAICharacter;

UCLASS()
class ARPG_PROJECT_API AAISpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	AAISpawnPoint();

	UFUNCTION(BlueprintCallable, Category = "AI | Spawn")
	void SpawnBasicActors();

	UFUNCTION(BlueprintCallable, Category = "AI | Spawn")
	void InitializeAI();

	UFUNCTION(BlueprintCallable, Category = "AI | Spawn")
	void InitializeAIAtIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "AI | Spawn")
	void SpawnMultipleAI();

	UFUNCTION(BlueprintPure, Category = "AI | Spawn")
	int32 GetSpawnedAICount() const { return SpawnedAICharacters.Num(); }

	UFUNCTION(BlueprintPure, Category = "AI | Spawn")
	TArray<AInGameAICharacter*> GetSpawnedAICharacters() const { return SpawnedAICharacters; }

	UFUNCTION(BlueprintPure, Category = "AI | Spawn")
	TArray<AInGameAIController*> GetSpawnedAIControllers() const { return SpawnedAIControllers; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "AI | Spawn")
	FVector FindValidSpawnLocation(int32 SpawnIndex, int32 MaxAttempts = 10);

	UFUNCTION(BlueprintPure, Category = "AI | Spawn")
	bool IsLocationValid(const FVector& Location) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | SpawnConfig")
	FAIManifest AISpawnConfig;

	/** Number of AI characters to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | SpawnConfig", meta = (ClampMin = "1", ClampMax = "50"))
	int32 SpawnCount = 1;

	/** Radius within which to spawn AI characters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | SpawnConfig", meta = (ClampMin = "0"))
	float SpawnRadius = 500.0f;

	/** Minimum distance between spawned AI characters to prevent overlap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | SpawnConfig", meta = (ClampMin = "0"))
	float MinDistanceBetweenAI = 200.0f;

	/** Height offset for line trace when finding spawn locations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | SpawnConfig")
	float TraceHeight = 1000.0f;

	/** Whether to randomize rotation for each spawned AI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI | SpawnConfig")
	bool bRandomizeRotation = false;

private:
	void InitializeAICharacter(int32 Index);

	void InitializeAIController(int32 Index);

	UFUNCTION(Client, Reliable)
	void ClientSpawnCharacter(int32 Index);

	UPROPERTY()
	TArray<TObjectPtr<AInGameAICharacter>> SpawnedAICharacters;

	UPROPERTY()
	TArray<TObjectPtr<AInGameAIController>> SpawnedAIControllers;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedPatrolPaths;

	/** Cached spawn locations to prevent overlap */
	TArray<FVector> UsedSpawnLocations;

	AActor* AIPatrollingPath = nullptr;
};
