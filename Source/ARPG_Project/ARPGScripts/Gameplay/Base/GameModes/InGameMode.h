// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/FARPGEventData.h"
#include "ARPGScripts/Gameplay/Base/GameModes/ARPGGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "InGameMode.generated.h"

class AAISpawnPoint;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API AInGameMode : public AARPGGameMode
{
	GENERATED_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
	virtual void BeginPlay() override;
	
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Travel")
	void Server_ReturnToLobby();

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void RequestRespawn(AController* Controller, float Delay = 0.0f);
	// AI start
	UFUNCTION(Server,Reliable)
	void RegisterAISpawnPoint(AAISpawnPoint* SpawnPoint);

	UFUNCTION(Server,Reliable)
	void UnRegisterAISpawnPoint(AAISpawnPoint* SpawnPoint);
	// AI end

	// TODO : use config data instead of hard coding
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameMode | MapConfig")
	FString IngameMapID;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GameMode | MapConfig")
	FString LobbyMapID;
	
protected:
	TSubclassOf<APawn> GetPlayerPawnClassFromGameInstance(AController* NewPlayer);

	FTransform GetPlayerPawnTransformFromGameInstance(AController* NewPlayer);

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	UFUNCTION()
	void SpawnPortals();

	void PerformRespawn(AController* Controller);
private:
	TArray<AAISpawnPoint*> AISpawnPoints;

	FTimerHandle RespawnTimerHandle;
};
