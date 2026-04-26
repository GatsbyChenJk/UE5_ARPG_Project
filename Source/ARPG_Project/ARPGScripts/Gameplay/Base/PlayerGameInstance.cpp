// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerGameInstance.h"
#include "DataTableUtils.h"
#include "ARPGScripts/Gameplay/Character/Lobby/Multiplayer/PlayerSessionManager.h"
#include "ARPGScripts/Gameplay/Character/Lobby/ShoppingSystem/ShopItemManifest.h"
#include "ARPGScripts/Gameplay/Character/SpecialOperations/FOperationManifest.h"
#include "ARPGScripts/Gameplay/GameMap/MapPointManager.h"

UPlayerGameInstance::UPlayerGameInstance()
{
	LoadedCharacterDataTable = nullptr;
	LoadedLevelConfigDataTable = nullptr;
	LoadedOperationDataTable = nullptr;
	LoadedWeaponDataTable = nullptr;
	LoadedWidgetDataTable = nullptr;
	LoadedShopSystemDataTable = nullptr;
	// MapManifest = new FMapManifest();
}

void UPlayerGameInstance::Init()
{
	Super::Init();

	InitMapManager();
}

void UPlayerGameInstance::Shutdown()
{
	Super::Shutdown();

	if (SessionManager)
	{
		SessionManager->RemoveFromRoot();
		SessionManager = nullptr;
	}

	if (MapManager)
	{
		MapManager->RemoveFromRoot();
		MapManager = nullptr;
	}
}

void UPlayerGameInstance::LoadOperationDataTableAsync()
{
	LoadDataTableAsyncTemplate(
	OperationDataTableSoftPtr,
	LoadedOperationDataTable,
	TEXT("Operation DataTable path is invalid!"),
	OnOperationDataTableLoaded,
	&ThisClass::OnOperationDataTableLoadedCallBack);
}

void UPlayerGameInstance::LoadWidgetDataTableAsync()
{
	LoadDataTableAsyncTemplate(
		WidgetDataTableSoftPtr,
		LoadedWidgetDataTable,
		TEXT("Widget DataTable path is invalid!"),
		OnWidgetDataTableLoaded,
		&ThisClass::OnWidgetConfigDataTableLoadedCallBack);		
}

FOperationManifest UPlayerGameInstance::GetOperationManifestByID(const FString& OperationID)
{
	return GetDataManifestByID<FOperationManifest>(
		LoadedOperationDataTable,
		TEXT("Operation DataTable path is invalid!"),
		OperationID);
}

FUIWidgetConfig UPlayerGameInstance::GetLoadedUIConfigByID(const FString& WidgetID)
{
	return GetDataManifestByID<FUIWidgetConfig>(
		LoadedWidgetDataTable,
		 TEXT("Widget DataTable is invalid!"),
		 WidgetID
		 );
}

void UPlayerGameInstance::LoadWeaponDataTableAsync()
{
	LoadDataTableAsyncTemplate(
		WeaponDataTableSoftPtr,
		LoadedWeaponDataTable,
		TEXT("LevelConfig DataTable path is invalid!"),
		OnWeaponDataTableLoaded,
		&ThisClass::OnWeaponDataTableLoadedCallBack);
}

bool UPlayerGameInstance::LoadWeaponDataFromTable(const FString& InWeaponID)
{
	if (!IsValid(LoadedWeaponDataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("Map DataTable is invalid!"));
		return false;
	}

	// 在数据表中查找对应ID的行
	if (FWeaponManifest* Manifest = LoadedWeaponDataTable->FindRow<FWeaponManifest>(*InWeaponID,TEXT("Debug Logs")))
	{
		WeaponData = *Manifest;
		UE_LOG(LogTemp, Warning, TEXT("Weapon Config Data is set, WeaponName:%s,NetRole:%d"),
			*Manifest->WeaponName,
			GetWorld()->GetNetMode());
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Weapon ID %s not found in DataTable!"), *InWeaponID);
	return false;
}

FWeaponManifest UPlayerGameInstance::GetWeaponManifestByID(const FString& InWeaponID)
{
	return GetDataManifestByID<FWeaponManifest>(
		LoadedWeaponDataTable,
		 TEXT("Weapon DataTable is invalid!"),
		 InWeaponID
		 );
}

FShopItemManifest UPlayerGameInstance::GetShopItemManifestByID(const FString& InShopID)
{
	return GetDataManifestByID<FShopItemManifest>(
		LoadedShopSystemDataTable,
		TEXT("ShopItem DataTable is invalid!"),
		InShopID);
}

bool UPlayerGameInstance::LoadLevelConfigDataFromTable(const FString& InMapID)
{
	if (!IsValid(LoadedLevelConfigDataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("Map DataTable is invalid!"));
		return false;
	}

	// 在数据表中查找对应ID的行
	if (FMapManifest* Manifest = LoadedLevelConfigDataTable->FindRow<FMapManifest>(*InMapID,TEXT("Debug Logs")))
	{
		MapManifest = *Manifest;
		UE_LOG(LogTemp, Warning, TEXT("Loaded Level Config Data is set, LevelPath:%s,GameModeClass Name:%s,NetRole:%d"),
			*Manifest->GetLevelPath(),
			*Manifest->GetGameModeClass(),
			GetWorld()->GetNetMode());
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Map ID %s not found in DataTable!"), *InMapID);
	return false;
}

void UPlayerGameInstance::LoadLevelConfigDataTableAsync()
{
	LoadDataTableAsyncTemplate(
		LevelConfigDataTableSoftPtr,
		LoadedLevelConfigDataTable,
		TEXT("LevelConfig DataTable path is invalid!"),
		OnMapDataTableLoaded,
		&ThisClass::OnLevelConfigDataTableLoadedCallBack);
}

void UPlayerGameInstance::Server_SetMapManifest_Implementation(const FString& InMapID)
{
	LoadLevelConfigDataFromTable(InMapID);
}

FMapManifest UPlayerGameInstance::GetMapManifestByID(const FString& InMapID)
{
	return GetDataManifestByID<FMapManifest>(
		LoadedLevelConfigDataTable,
		TEXT("Level Config DataTable is invalid!"),
		InMapID
		);
}

void UPlayerGameInstance::LoadCharacterDataTableAsync()
{
	LoadDataTableAsyncTemplate(
		CharacterDataTablePtr,
		LoadedCharacterDataTable,
		TEXT("Character DataTable path is invalid!"),
		OnCharacterDataTableLoaded,
		&ThisClass::OnCharacterDataTableLoadedCallBack);
}

void UPlayerGameInstance::LoadShopSystemDataTableAsync()
{
	LoadDataTableAsyncTemplate(
		ShopSystemDataTablePtr,
		LoadedShopSystemDataTable,
		TEXT("ShopSystem DataTable path is invalid!"),
		OnShopSystemDataTableLoaded,
		&ThisClass::OnShopSystemDataTableLoadedCallBack);
}

void UPlayerGameInstance::Server_SetCharacterData_Implementation(const FString& InCharacterID)
{
	LoadCharacterDataFromTable(InCharacterID);
}

bool UPlayerGameInstance::LoadCharacterDataFromTable(const FString& InCharacterID)
{
	if (!IsValid(LoadedCharacterDataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character DataTable is invalid!"));
		return false;
	}

	// 在数据表中查找对应ID的行
	if (FCharacterManifest* Manifest = LoadedCharacterDataTable->FindRow<FCharacterManifest>(*InCharacterID,TEXT("Debug Logs")))
	{
		CharacterData.SetCharacterManifest(Manifest);
		CharacterData.bIsCharacterSet = true;
		OnCharacterChosen.Broadcast();
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Character ID %s not found in DataTable!"), *InCharacterID);
	return false;
}

// bool UPlayerGameInstance::LoadShopSystemDataFromTable(const FString& InShopID)
// {
// 	
// }

TArray<FCharacterData*> UPlayerGameInstance::GetAllCharacterData() const
{
	TArray<FCharacterData*> AllData;
	if (LoadedCharacterDataTable)
	{
		TArray<FCharacterManifest*> AllRawData;
		LoadedCharacterDataTable->GetAllRows<FCharacterManifest>(TEXT(""), AllRawData);
		AllData.Reserve(AllRawData.Num());
		for (FCharacterManifest* Manifest : AllRawData)
		{
			FCharacterData* Data = new FCharacterData();
			Data->SetCharacterManifest(Manifest);
			AllData.Add(MoveTemp(Data));
		}
	}
	return AllData;
}

FCharacterData UPlayerGameInstance::GetCharacterDataByID(const FString& CharacterID)
{
	if (!IsValid(LoadedCharacterDataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character DataTable is invalid!"));
		return {};
	}
	
	// 在数据表中查找对应ID的行
	if (FCharacterManifest* Manifest = LoadedCharacterDataTable->FindRow<FCharacterManifest>(*CharacterID,TEXT("Debug Logs")))
	{
		FCharacterData Data;
		Data.SetCharacterManifest(Manifest);
		//CharacterData.bIsCharacterSet = true;
		//OnCharacterChosen.Broadcast();
		
		return Data;
	}
	
	return {};
}

void UPlayerGameInstance::OnLevelConfigDataTableLoadedCallBack()
{
	LoadedLevelConfigDataTable = LevelConfigDataTableSoftPtr.Get();
	// broadcast delegate when is needed
	if (LoadedLevelConfigDataTable)
	{
		UE_LOG(LogTemp, Log, TEXT("Map DataTable loaded successfully!"));
		OnMapDataTableLoaded.Broadcast(); // 通知UI等系统
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Map DataTable!"));
	}
	//
	DataTableLoadHandle.Reset();
}

void UPlayerGameInstance::OnCharacterDataTableLoadedCallBack()
{
	// 获取已加载的对象
	LoadedCharacterDataTable = CharacterDataTablePtr.Get();
	if (LoadedCharacterDataTable)
	{
		UE_LOG(LogTemp, Log, TEXT("Character DataTable loaded successfully!"));
		OnCharacterDataTableLoaded.Broadcast(); // 通知UI等系统
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Character DataTable!"));
	}
	DataTableLoadHandle.Reset(); // 清理句柄
}

void UPlayerGameInstance::OnWeaponDataTableLoadedCallBack()
{
	LoadedWeaponDataTable = WeaponDataTableSoftPtr.Get();
	// broadcast delegate when is needed
	
	//
	DataTableLoadHandle.Reset();
}

void UPlayerGameInstance::OnWidgetConfigDataTableLoadedCallBack()
{
	LoadedWidgetDataTable = WidgetDataTableSoftPtr.Get();

	if (LoadedWidgetDataTable)
	{
		UE_LOG(LogTemp, Log, TEXT("Widget DataTable loaded successfully!"));
		OnWidgetDataTableLoaded.Broadcast(); // 通知UI等系统
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Widget DataTable!"));
	}
	
	DataTableLoadHandle.Reset();
}

void UPlayerGameInstance::OnOperationDataTableLoadedCallBack()
{
	LoadedOperationDataTable = OperationDataTableSoftPtr.Get();

	if (LoadedOperationDataTable)
	{
		UE_LOG(LogTemp, Log, TEXT("Operation DataTable loaded successfully!"));
		OnOperationDataTableLoaded.Broadcast(); // 通知UI等系统
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Operation DataTable!"));
	}
	
	DataTableLoadHandle.Reset();
}

void UPlayerGameInstance::OnShopSystemDataTableLoadedCallBack()
{
	LoadedShopSystemDataTable = ShopSystemDataTablePtr.Get();

	if (LoadedShopSystemDataTable)
	{
		UE_LOG(LogTemp, Log, TEXT("ShopItem DataTable loaded successfully!"));
		OnShopSystemDataTableLoaded.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ShopItem DataTable!"));
	}

	DataTableLoadHandle.Reset();
}

void UPlayerGameInstance::InitSessionManager(APlayerController* PC)
{
	if (!SessionManager)
	{
		SessionManager = NewObject<UPlayerSessionManager>();
		SessionManager->AddToRoot();
	}

	SessionManager->OwningGameInstance = this;
	SessionManager->OwningPlayerController = PC;

	// 初始化OSS（仅首次）
	if (!SessionManager->SessionInterface.IsValid() && IOnlineSubsystem::Get())
	{
		SessionManager->SessionInterface = IOnlineSubsystem::Get()->GetSessionInterface();
		SessionManager->BindOSSCallbacks();
	}
}

void UPlayerGameInstance::InitMapManager()
{
	if (!MapManager)
	{
		MapManager = NewObject<UMapPointManager>();
		MapManager->AddToRoot();
	}
}


