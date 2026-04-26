// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PlayerSessionManager.generated.h"


UCLASS()
class ARPG_PROJECT_API UPlayerSessionManager : public UObject
{
	GENERATED_BODY()

public:
	//声明会话创建完成时的委托
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	//声明查找会话完成时的委托
	FOnFindSessionsCompleteDelegate FindSessionCompleteDelegate;
	//加入会话结束时的委托
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;

	// ========== 核心接口（供UI调用）==========
	
	void CreateLANSession(const FString& RoomName, int32 MaxPlayers = 4);
	
	void FindLANSessions();
	
	void JoinLANSession(const FOnlineSessionSearchResult& SelectedSession);
	
	void DestroySession();

	TSharedPtr<FOnlineSessionSearch> GetSessionSearch() {return SessionSearch;}

	IOnlineSessionPtr GetSessionInterface() {return SessionInterface;};
	
	void TravelToIngameLevel(APlayerController* PC);

public:
	// OSS会话接口
	IOnlineSessionPtr SessionInterface;
	
	// 搜索结果缓存
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	// 游戏实例引用（单例持有）
	UGameInstance* OwningGameInstance = nullptr;

	APlayerController* OwningPlayerController = nullptr;

	// ========== OSS原生委托绑定 ==========
	void BindOSSCallbacks();
private:
	FString TravelMapPath;
	
	// 创建会话回调（OSS原生）
	void OnCreateSessionComplete_Func(FName SessionName, bool bSuccess);
	// 搜索会话回调（OSS原生）
	void OnFindSessionsComplete_Func(bool bSuccess);
	// 加入会话回调（OSS原生）
	void OnJoinSessionComplete_Func(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
};
