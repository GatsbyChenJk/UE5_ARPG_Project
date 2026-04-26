// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGSaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ARPGSaveGameManager.generated.h"

class USaveGame;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveComplete, bool, bSuccess, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoadComplete, bool, bSuccess, int32, SlotIndex,UARPGSaveGame*,SaveGame);

UCLASS()
class ARPG_PROJECT_API UARPGSaveGameManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 异步保存到指定槽位
	void AsyncSaveGame(const FARPGPlayerData& InData,int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void AsyncSaveGame(int32 SlotIndex);

	// 异步加载指定槽位
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void AsyncLoadGame(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SynchronousSaveGame(const FARPGPlayerData& InData,int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	UARPGSaveGame* SynchronousLoadGame(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SetPlayerData(FARPGPlayerData InData);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	FARPGPlayerData GetPlayerData();
	
	// 事件
	UPROPERTY(BlueprintAssignable)
	FOnSaveComplete OnSaveCompleted;

	UPROPERTY(BlueprintAssignable)
	FOnLoadComplete OnLoadCompleted;

private:
	FARPGPlayerData CurrentPlayerData;
	int32 PendingSlotIndex = -1;

	FString GetSlotName(int32 SlotIndex) const;
	
	void HandleSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess);
	void HandleLoadComplete(const FString& SlotName, int32 UserIndex, USaveGame* LoadedSaveGame);
};
