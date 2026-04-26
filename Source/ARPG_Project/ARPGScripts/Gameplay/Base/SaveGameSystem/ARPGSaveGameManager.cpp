// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGSaveGameManager.h"

#include "ARPGSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UARPGSaveGameManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentPlayerData = FARPGPlayerData();
}

void UARPGSaveGameManager::AsyncSaveGame(const FARPGPlayerData& InData, int32 SlotIndex)
{
	SetPlayerData(InData);
	
	AsyncSaveGame(SlotIndex);
}

void UARPGSaveGameManager::AsyncSaveGame(int32 SlotIndex)
{
	// 创建存档对象并填充当前数值
	UARPGSaveGame* SaveGame;
	if (UGameplayStatics::DoesSaveGameExist(GetSlotName(SlotIndex),0))
	{
		SaveGame = Cast<UARPGSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(SlotIndex), 0));
	}
	else
	{
		SaveGame = Cast<UARPGSaveGame>(UGameplayStatics::CreateSaveGameObject(UARPGSaveGame::StaticClass()));
	}
	
	SaveGame->PlayerData = CurrentPlayerData;
	// TODO:get player data from gameInstance
	
	FAsyncSaveGameToSlotDelegate SaveDelegate;
	SaveDelegate.BindUObject(this, &ThisClass::HandleSaveComplete);
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, GetSlotName(SlotIndex), 0, SaveDelegate);

	PendingSlotIndex = SlotIndex;
}

void UARPGSaveGameManager::AsyncLoadGame(int32 SlotIndex)
{
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindUObject(this, &ThisClass::HandleLoadComplete);
	UGameplayStatics::AsyncLoadGameFromSlot(GetSlotName(SlotIndex), 0, LoadDelegate);

	PendingSlotIndex = SlotIndex;
}

void UARPGSaveGameManager::SynchronousSaveGame(const FARPGPlayerData& InData, int32 SlotIndex)
{
	SetPlayerData(InData);

	UARPGSaveGame* SaveGame;
	if (UGameplayStatics::DoesSaveGameExist(GetSlotName(SlotIndex),0))
	{
		SaveGame = Cast<UARPGSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(SlotIndex), 0));
	}
	else
	{
		SaveGame = Cast<UARPGSaveGame>(UGameplayStatics::CreateSaveGameObject(UARPGSaveGame::StaticClass()));
	}

	SaveGame->PlayerData = CurrentPlayerData;
	bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, GetSlotName(SlotIndex), 0);
	if (!bSuccess)
	{
		UE_LOG(LogTemp,Log,TEXT("UARPGSaveGameManager::SynchronousSaveGame save game failed"))
	}

	OnSaveCompleted.Broadcast(bSuccess,SlotIndex);
}

UARPGSaveGame* UARPGSaveGameManager::SynchronousLoadGame(int32 SlotIndex)
{
	UARPGSaveGame* SaveGame = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(GetSlotName(SlotIndex),0))
	{
		SaveGame = Cast<UARPGSaveGame>(UGameplayStatics::LoadGameFromSlot(GetSlotName(SlotIndex), 0));
		SetPlayerData(SaveGame->PlayerData);
	}
	return SaveGame;
}

void UARPGSaveGameManager::SetPlayerData(FARPGPlayerData InData)
{
	CurrentPlayerData = InData;
}

FARPGPlayerData UARPGSaveGameManager::GetPlayerData()
{
	return CurrentPlayerData;
}

FString UARPGSaveGameManager::GetSlotName(int32 SlotIndex) const
{
	return FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
}

void UARPGSaveGameManager::HandleSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess)
{
	OnSaveCompleted.Broadcast(bSuccess, PendingSlotIndex);
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Save failed for slot %s"), *SlotName);
	}
}

void UARPGSaveGameManager::HandleLoadComplete(const FString& SlotName, int32 UserIndex, USaveGame* LoadedSaveGame)
{
	bool bSuccess = false;
	UARPGSaveGame* SaveGame = Cast<UARPGSaveGame>(LoadedSaveGame);
	if (IsValid(SaveGame))
	{
		CurrentPlayerData = SaveGame->PlayerData;
		bSuccess = true;
	}

	OnLoadCompleted.Broadcast(bSuccess, PendingSlotIndex,SaveGame);
}
