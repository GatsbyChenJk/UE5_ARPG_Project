// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ARPGScripts/Gameplay/Weapon/FWeaponManifest.h"
#include "ARPGScripts/Gameplay/Character/FCharacterManifest.h"
#include "ARPGScripts/Gameplay/GameMap/FMapManifest.h"
#include "ARPGScripts/Widgets/Base/FWidgetManifest.h"
#include "Engine/StreamableManager.h"
#include "PlayerGameInstance.generated.h"

/**
 * 
 */

struct FShopItemManifest;
class UMapPointManager;
struct FOperationManifest;
class UPlayerSessionManager;
struct FStreamableHandle;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDataTableLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterChosen);

UCLASS()
class ARPG_PROJECT_API UPlayerGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UPlayerGameInstance();

public:
	virtual void Init() override;
	virtual void Shutdown() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Operation Config")
	TSoftObjectPtr<UDataTable> OperationDataTableSoftPtr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Config")
	TSoftObjectPtr<UDataTable> WidgetDataTableSoftPtr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Config")
	TSoftObjectPtr<UDataTable> WeaponDataTableSoftPtr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map Selection")
	TSoftObjectPtr<UDataTable> LevelConfigDataTableSoftPtr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Character Selection")
	TSoftObjectPtr<UDataTable> CharacterDataTablePtr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Shop System")
	TSoftObjectPtr<UDataTable> ShopSystemDataTablePtr;

	FOnDataTableLoaded OnOperationDataTableLoaded;
	
	FOnDataTableLoaded OnWidgetDataTableLoaded;
	
	FOnDataTableLoaded OnMapDataTableLoaded;

	FOnDataTableLoaded OnWeaponDataTableLoaded;
	
	FOnDataTableLoaded OnCharacterDataTableLoaded;
	
	FOnCharacterChosen OnCharacterChosen;

	FOnDataTableLoaded OnShopSystemDataTableLoaded;
	
	// async load start
	void LoadOperationDataTableAsync();
	
	void LoadWidgetDataTableAsync();
	
	void LoadLevelConfigDataTableAsync();
	
	void LoadWeaponDataTableAsync();
	
	void LoadCharacterDataTableAsync();

	void LoadShopSystemDataTableAsync();

	template<typename CallbackFunc>
   void LoadDataTableAsyncTemplate(const TSoftObjectPtr<UDataTable>& SoftTablePtr, 
								   UDataTable*& OutLoadedTablePtr,
								   const FString& ErrorLog,
								   FOnDataTableLoaded LoadedDelegate,
								   CallbackFunc&& LoadedCallback);
	// async load end
	
	// getter start
	FOperationManifest GetOperationManifestByID(const FString& OperationID);
	
	FUIWidgetConfig GetLoadedUIConfigByID(const FString& WidgetID);
	
	FWeaponManifest GetWeaponManifest() const {return WeaponData;}
    
	FWeaponManifest GetWeaponManifestByID(const FString& InWeaponID);

	FShopItemManifest GetShopItemManifestByID(const FString& InShopID);
	
	FMapManifest GetMapManifest() const { return MapManifest; }
                                                                         
	FMapManifest GetMapManifestByID(const FString& InMapID);

	FCharacterData GetCharacterData() { return CharacterData; }

	TArray<FCharacterData*> GetAllCharacterData() const;

	FCharacterData GetCharacterDataByID(const FString& CharacterID);

	template<typename DataManifest>
	DataManifest GetDataManifestByID(UDataTable* SearchTable,const FString& ErrorLog,const FString& InSearchID);
    
	template<typename DataManifest,typename FuncT>
	DataManifest GetDataManifestByID(UDataTable* SearchTable,const FString& ErrorLog,const FString& InSearchID,const FuncT& SetterFunc);
	
	// getter end

	// setter start
	void SetWeaponManifest(const FWeaponManifest& NewManifest)
	{
		WeaponData = NewManifest;
	}

	void SetMapManifest(const FMapManifest& NewManifest)
	{
		MapManifest = NewManifest;
	}

	void SetCharacterData(const FCharacterData& InData)
	{
		CharacterData = InData;
		CharacterData.bIsCharacterSet = true;
	}
	// setter end

	// loader and RPCs start
	bool LoadWeaponDataFromTable(const FString& InWeaponID);

	bool LoadLevelConfigDataFromTable(const FString& InMapID);
	
	bool LoadCharacterDataFromTable(const FString& InCharacterID);

	//bool LoadShopSystemDataFromTable(const FString& InShopID);
	
	UFUNCTION(Server,Reliable)
	void Server_SetMapManifest(const FString& InMapID);
	
	UFUNCTION(Server,Reliable)
	void Server_SetCharacterData(const FString& InCharacterID);
	// loader and RPCs end

private:
	// data manifest start
	FMapManifest MapManifest;
	
	FCharacterData CharacterData;

	FWeaponManifest WeaponData;
	// data manifest end

	// async load start
	FStreamableManager StreamableManager;
	
	TSharedPtr<FStreamableHandle> DataTableLoadHandle;
	
	UPROPERTY(Transient)
	UDataTable* LoadedLevelConfigDataTable;
	
	UPROPERTY(Transient)
	UDataTable* LoadedCharacterDataTable;

	UPROPERTY(Transient)
	UDataTable* LoadedWeaponDataTable;

	UPROPERTY(Transient)
	UDataTable* LoadedWidgetDataTable;
	
	UPROPERTY(Transient)
	UDataTable* LoadedOperationDataTable;

	UPROPERTY(Transient)
	UDataTable* LoadedShopSystemDataTable;
	
	void OnLevelConfigDataTableLoadedCallBack();
	
	void OnCharacterDataTableLoadedCallBack();

	void OnWeaponDataTableLoadedCallBack();

	void OnWidgetConfigDataTableLoadedCallBack();

	void OnOperationDataTableLoadedCallBack();

	void OnShopSystemDataTableLoadedCallBack();
	// async load end
public:
	//multiplayer start
	UPROPERTY()
	UPlayerSessionManager* SessionManager = nullptr;

	UFUNCTION()
	void InitSessionManager(APlayerController* PC);
	//multiplayer end

	// Map points start
	UPROPERTY()
	UMapPointManager* MapManager = nullptr;

	void InitMapManager();
	// Map points end
};

template <typename CallbackFunc>
void UPlayerGameInstance::LoadDataTableAsyncTemplate(const TSoftObjectPtr<UDataTable>& SoftTablePtr,
	UDataTable*& OutLoadedTablePtr, const FString& ErrorLog, FOnDataTableLoaded LoadedDelegate,CallbackFunc&& LoadedCallback)
{
	// 不变逻辑1：已加载则直接广播回调并返回
	if (OutLoadedTablePtr)
	{
		if (LoadedDelegate.IsBound())
		{
			LoadedDelegate.Broadcast();
		}
		return;
	}

	// 不变逻辑2：获取并检查软引用路径有效性
	FSoftObjectPath TablePath = SoftTablePtr.ToSoftObjectPath();
	if (TablePath.IsValid())
	{
		// 不变逻辑3：发起异步加载请求
		DataTableLoadHandle = StreamableManager.RequestAsyncLoad(
			TablePath,
			FStreamableDelegate::CreateUObject(this, Forward<CallbackFunc>(LoadedCallback))
		);
	}
	else
	{
		// 可变日志信息
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorLog);
	}
}

template <typename DataManifest>
DataManifest UPlayerGameInstance::GetDataManifestByID(UDataTable* SearchTable,const FString& ErrorLog,const FString& InSearchID)
{
	if (!IsValid(SearchTable))
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorLog);
		return {};
	}

	if (DataManifest* Manifest = SearchTable->FindRow<DataManifest>(*InSearchID,TEXT("Debug Logs")))
	{
		DataManifest Data;
		Data = *Manifest;
		return Data;
	}

	return {};
}

template <typename DataManifest, typename FuncT>
DataManifest UPlayerGameInstance::GetDataManifestByID(UDataTable* SearchTable, const FString& ErrorLog,
	const FString& InSearchID, const FuncT& SetterFunc)
{
	if (!IsValid(SearchTable))
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorLog);
		return {};
	}

	if (DataManifest* Manifest = SearchTable->FindRow<DataManifest>(*InSearchID,TEXT("Debug Logs")))
	{
		DataManifest Data;
		SetterFunc(Manifest);
		return Data;
	}

	return {};
}
