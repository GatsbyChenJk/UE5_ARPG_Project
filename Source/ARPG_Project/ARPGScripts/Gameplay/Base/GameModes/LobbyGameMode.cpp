// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/GameMap/MapLoadingManager.h"
#include "Engine/World.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::Server_TravelToLevelByMapID_Implementation(const FString& InMapID)
{
	if (!HasAuthority()) return;
	if (UMapLoadingManager* MapLoader = GetGameInstance()->GetSubsystem<UMapLoadingManager>())
	{
		MapLoader->ServerTravelToMapByID(InMapID);
	}
}

void ALobbyGameMode::Server_TravelToConfigLevel_Implementation()
{
	if (!HasAuthority()) return;
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		if (UMapLoadingManager* MapLoader = GameInstance->GetSubsystem<UMapLoadingManager>())
		{
			MapLoader->ServerTravelToMap(GameInstance->GetMapManifest());
		}
	}
}
