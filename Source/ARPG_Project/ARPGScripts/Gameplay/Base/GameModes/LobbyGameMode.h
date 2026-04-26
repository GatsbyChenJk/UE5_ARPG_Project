// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Base/GameModes/ARPGGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API ALobbyGameMode : public AARPGGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(Server,Reliable)
	void Server_TravelToLevelByMapID(const FString& InMapID);
private:
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Travel")
	void Server_TravelToConfigLevel();
};
