#include "MapLoadingManager.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/FARPGEventData.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UMapLoadingManager::LoadMap(const FMapManifest& MapManifest)
{
	if (!MapManifest.IsValidMap()) return;

	PendingMapManifest = MapManifest;
	bIsMultiplayerTravel = false;

	StreamableManager.RequestAsyncLoad(
		MapManifest.LevelToOpen.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UMapLoadingManager::OnLevelLoadComplete)
	);
}

void UMapLoadingManager::OnLevelLoadComplete()
{
	// if (GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, TEXT("OnLevelLoadComplete: Starting 2s timer"));
	// }

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
	{
		return;
	}

	// Delay 2 seconds to ensure loading screen is visible
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, [this, World]()
	{
		// if (GEngine)
		// {
		// 	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Timer callback: Hiding screen and switching level"));
		// }
		//

		// Switch to target level
		if (bIsMultiplayerTravel)
		{
			// warning !!! 不能修改实现的值
			FString LevelPath = PendingMapManifest.LevelToOpen.GetAssetName();
			// warning !!!
			FString URL = FString::Printf(TEXT("%s?Game=%s?Listen"), *LevelPath, *PendingMapManifest.GetGameModeClass());
			World->ServerTravel(URL);
		}
		else
		{
			FString LevelPath = PendingMapManifest.GetLevelPath();
			UGameplayStatics::OpenLevel(World, FName(*LevelPath));
		}
	}, 2.0f, false);
}

void UMapLoadingManager::ServerTravelToMap(const FMapManifest& MapManifest)
{
	if (!MapManifest.IsValidMap()) return;

	PendingMapManifest = MapManifest;
	bIsMultiplayerTravel = true;

	StreamableManager.RequestAsyncLoad(
		MapManifest.LevelToOpen.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &UMapLoadingManager::OnLevelLoadComplete)
	);
}

void UMapLoadingManager::ServerTravelToMapByID(const FString& MapID)
{
	UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance());
	if (!GameInstance) return;

	FMapManifest Manifest = GameInstance->GetMapManifestByID(MapID);
	ServerTravelToMap(Manifest);
}

