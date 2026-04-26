// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerSessionManager.h"
#include "OnlineSessionSettings.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/PlayerController/LobbyController.h"

void UPlayerSessionManager::CreateLANSession(const FString& RoomName, int32 MaxPlayers)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}
	
	//通过NAME_GameSession创建一个临时会话
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		//销毁之前的会话
		SessionInterface->DestroySession(NAME_GameSession);
	}
	//将委托添加到句柄中
	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
 
	//修改游戏会话设置
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	//如果不使用steam的话就用本地连接
	SessionSettings->bIsLANMatch = true;  // 重要：对于Steam设为false
	SessionSettings->NumPublicConnections = MaxPlayers;
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = false;
	SessionSettings->bIsDedicated = false;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bShouldAdvertise = true;
	
	SessionSettings->Set(FName("RoomName"), RoomName, EOnlineDataAdvertisementType::ViaOnlineService);

	if (IsValid(OwningGameInstance))
	{
		UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(OwningGameInstance);
		auto Manifest = GameInstance->GetMapManifest();
		
		if (!Manifest.IsValidMap())
		{
			Manifest = GameInstance->GetMapManifestByID("M02");
		}
		
		FString MapPath = Manifest.LevelToOpen.GetAssetName();
		SessionSettings->Set(FName("MapPath"),MapPath,EOnlineDataAdvertisementType::ViaOnlineService);
	}
	
	
	if (IsValid(OwningPlayerController))
	{
		//TODO : implement login logic for uniqueid
		// SessionInterface->CreateSession(OwningPlayerController->GetUniqueID(), NAME_GameSession, *SessionSettings);
		SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings);
	}
}

void UPlayerSessionManager::FindLANSessions()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}
 
	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegate);
 
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true; // LAN 搜索
	SessionSearch->MaxSearchResults = 10;
	SessionSearch->TimeoutInSeconds = 50.0f;

	// 替换废弃的 SEARCH_PRESENCE，筛选有 Presence 的会话
	SessionSearch->QuerySettings.Set(FName("Presence"), true, EOnlineComparisonOp::Equals);
	
	if (IsValid(OwningPlayerController))
	{
		// SessionInterface->FindSessions(OwningPlayerController->GetUniqueID(), SessionSearch.ToSharedRef());
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UPlayerSessionManager::JoinLANSession(const FOnlineSessionSearchResult& SelectedSession)
{
	if (!SessionInterface.IsValid() || !SelectedSession.IsValid()) return;
	
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	if (IsValid(OwningPlayerController))
	{
		SelectedSession.Session.SessionSettings.Get(FName("MapPath"),TravelMapPath);
		
		SessionInterface->JoinSession(0, FName(NAME_GameSession), SelectedSession);
	}
}

void UPlayerSessionManager::DestroySession()
{
	if (!SessionInterface.IsValid()) return;

	SessionInterface->DestroySession(FName(NAME_GameSession));
	UE_LOG(LogTemp, Log, TEXT("销毁/退出房间成功"));
}

void UPlayerSessionManager::BindOSSCallbacks()
{
	// 绑定OSS创建会话回调
	CreateSessionCompleteDelegate.BindUObject(this, &UPlayerSessionManager::OnCreateSessionComplete_Func);
	// 绑定OSS搜索会话回调
	FindSessionCompleteDelegate.BindUObject(this, &UPlayerSessionManager::OnFindSessionsComplete_Func);
	// 绑定OSS加入会话回调
	JoinSessionCompleteDelegate.BindUObject(this, &UPlayerSessionManager::OnJoinSessionComplete_Func);
}

void UPlayerSessionManager::OnCreateSessionComplete_Func(FName SessionName, bool bSuccess)
{
	if (bSuccess)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Blue,
				FString::Printf(TEXT("Created Session: %s"), *SessionName.ToString())
			);
		}
		
		if (OwningPlayerController)
		{
			APlayerController* PlayerController = Cast<APlayerController>(OwningPlayerController);
			if (ALobbyController* LobbyController = Cast<ALobbyController>(PlayerController))
			{
				if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(OwningGameInstance))
				{
					auto Manifest = GI->GetMapManifest();
					FString MapID = Manifest.MapID;

					LobbyController->Server_TravelToLevel(MapID.IsEmpty() ? FString("M02") : MapID);
				}
			}
			
		}
		
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Red,
				FString(TEXT("Fail to create session")));
		}
	}
}

void UPlayerSessionManager::OnFindSessionsComplete_Func(bool bSuccess)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}
	//need to test with two processes
	//process 1 : listen server (packaged)
	//process 2 : client
	// and test with player id 0
	for (FOnlineSessionSearchResult Result : SessionSearch->SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Cyan,
				FString::Printf(TEXT("Id: %s ,User: %s"),*Id, *User)
			);
		}
	}
}

void UPlayerSessionManager::OnJoinSessionComplete_Func(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;

	FString Address;
	if (SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Cyan,
				FString::Printf(TEXT("Connect String : %s/%s"),*Address,*TravelMapPath));
		}

		
		//TODO:Call Travel Func
		//need to test in same env
		//eg.same pak version
		if (IsValid(OwningPlayerController) && IsValid(OwningGameInstance))
		{
			FString URL = FString::Printf(TEXT("%s/%s"),*Address,*TravelMapPath);
			OwningPlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			
		}
	}
}
