#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ARPGScripts/Gameplay/GameMap/FMapManifest.h"
#include "Engine/StreamableManager.h"
#include "MapLoadingManager.generated.h"


UCLASS()
class ARPG_PROJECT_API UMapLoadingManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 加载地图并显示加载界面，MapManifest 包含关卡路径和地图信息
	UFUNCTION(BlueprintCallable, Category = "MapLoading")
	void LoadMap(const FMapManifest& MapManifest);

	// 通过 MapManifest 执行 ServerTravel（仅 Server 调用）
	UFUNCTION(BlueprintCallable, Category = "MapLoading")
	void ServerTravelToMap(const FMapManifest& MapManifest);

	// 通过 MapID 查找 Manifest 后执行 ServerTravel（仅 Server 调用）
	UFUNCTION(BlueprintCallable, Category = "MapLoading")
	void ServerTravelToMapByID(const FString& MapID);

private:
	void OnLevelLoadComplete();

	FStreamableManager StreamableManager;
	FMapManifest PendingMapManifest;
	bool bIsMultiplayerTravel = false;
};
