#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/GameModeBase.h"
#include "FMapManifest.generated.h"

USTRUCT(BlueprintType)
struct FBasePointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform PointTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEnabled;
	// 概率
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SelectWeight;
};

USTRUCT(BlueprintType)
struct FSpawnPointData : public FBasePointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayerToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSelfCamp;
};

USTRUCT(BlueprintType)
struct FEscapePointData : public FBasePointData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNeedEscapeItem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayerToEscape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeWaitingTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> PortalClass;
};

USTRUCT(BlueprintType)
struct FMapManifest : public FTableRowBase
{
	GENERATED_BODY()

	FMapManifest() {};

	FString GetLevelPath() const
	{
		if (!LevelToOpen.IsValid())
		{
			return FString();
		}
		return LevelToOpen.ToSoftObjectPath().ToString();
	}

	FString GetGameModeClass() const
	{
		if (!IsValid(GameModeClassForLevel))
		{
			return FString();
		}
		return GameModeClassForLevel->GetPathName();
	}

	bool IsValidMap() const
	{
		return IsValid(GameModeClassForLevel);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Level")
	FString MapID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSoftObjectPtr<UWorld> LevelToOpen; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TSubclassOf<AGameModeBase> GameModeClassForLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	FString GameModeName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	TObjectPtr<UTexture2D> MapIcon;

	// in-game config start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level | Ingame")
	TArray<FSpawnPointData> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level | Ingame")
	TArray<FEscapePointData> EscapePoints;
	// in-game config end
};
