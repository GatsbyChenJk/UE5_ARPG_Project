// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerSessionSelect.h"

#include "OnlineSessionSettings.h"
#include "PlayerSessionManager.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Widgets/Base/WidgetManager.h"

void UPlayerSessionSelect::NativeConstruct()
{
	Super::NativeConstruct();

	if (const auto WidgetSubSystem = GetGameInstance()->GetSubsystem<UWidgetManager>())
	{
		WidgetSubSystem->RegisterWidget(FString("1003"),this);
	}
}

void UPlayerSessionSelect::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GI->InitSessionManager(GetOwningPlayer());
		SessionMgr = GI->SessionManager;
		if (SessionMgr)
		{
			CreateSessionDelegateHandle.BindUObject(this,&ThisClass::OnCreateSessionResult);
			FindSessionDelegateHandle.BindUObject(this,&ThisClass::OnFindSessionsResult);
			JoinSessionDelegateHandle.BindUObject(this,&ThisClass::OnJoinSessionResult);
		}
	}

	OnSessionItemSelected.AddDynamic(this,&ThisClass::OnSessionItemSelect);
	ListView_RoomInfo->ClearListItems();
}


void UPlayerSessionSelect::OnCreateRoomBtnClicked(const FString& RoomName)
{
	if (SessionMgr)
	{
		SessionMgr->CreateLANSession(RoomName);
		SessionMgr->GetSessionInterface()->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
	}
}

void UPlayerSessionSelect::OnSearchRoomBtnClicked()
{
	if (SessionMgr)
	{
		ShowTips(FString(L"刷新中..."),false);
		Button_Refresh->SetIsEnabled(false);
		SessionMgr->FindLANSessions();
		SessionMgr->GetSessionInterface()->AddOnFindSessionsCompleteDelegate_Handle(FindSessionDelegateHandle);
	}
}

void UPlayerSessionSelect::OnJoinRoomBtnClicked(int32 SelectedIndex)
{
	if (SessionMgr && CachedSearchResults.IsValidIndex(SelectedIndex))
	{
		SessionMgr->JoinLANSession(CachedSearchResults[SelectedIndex]);
		SessionMgr->GetSessionInterface()->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
	}
}

void UPlayerSessionSelect::OnSessionItemSelect(UObject* Item)
{
	if (IsValid(Item))
	{
		int32 Index = ListView_RoomInfo->GetIndexForItem(Item);
		CurrentSelectedSession = Index;
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("UPlayerSessionSelect::OnSessionItemSelect NULL Item"));
	}
}

void UPlayerSessionSelect::ShowTips(const FString& Tip, bool bIsError)
{
	Text_Tips->SetText(FText::FromString(Tip));
	Text_Tips->SetColorAndOpacity(bIsError ? FLinearColor::Red : FLinearColor::Green);
}

void UPlayerSessionSelect::UpdateRoomList()
{
	ListView_RoomInfo->ClearListItems();
	for (const FOnlineSessionSearchResult& Result : CachedSearchResults)
	{
		USessionData* SessionInfo = NewObject<USessionData>();
		SessionInfo->AddToRoot();
		
		SessionInfo->MapName = FString("TestMap");

		FString RoomName;
		Result.Session.SessionSettings.Get(FName("RoomName"),RoomName);
		SessionInfo->RoomName = RoomName.IsEmpty() ? FString("Room") : RoomName;

		int32 CurrentAvailable = Result.Session.NumOpenPublicConnections;
		int32 MaxMember = Result.Session.SessionSettings.NumPublicConnections;
		int32 CurrentPlayers = MaxMember - CurrentAvailable;
		SessionInfo->MemberInfo = FString::Printf(TEXT("Num %d/%d"), CurrentPlayers,MaxMember);
		
		ListView_RoomInfo->AddItem(SessionInfo);
	}
}

void UPlayerSessionSelect::OnCreateSessionResult(FName SessionName,bool bSuccess)
{
	if (bSuccess)
	{
		ShowTips(FString(L"房间创建成功！等待其他玩家加入..."), false);
	}
	else
	{
		ShowTips(FString(L"房间创建失败！请检查网络或重试"), true);
	}
}

void UPlayerSessionSelect::OnFindSessionsResult(bool bSuccess)
{
	Button_Refresh->SetIsEnabled(true);
	CachedSearchResults.Empty();

	if (bSuccess)
	{
		TArray<FOnlineSessionSearchResult> Results = SessionMgr->GetSessionSearch()->SearchResults;
		if (Results.Num() > 0)
		{
			
			CachedSearchResults = Results;
			UpdateRoomList();
			ShowTips(FString::Printf(TEXT("搜索成功！找到 %d 个局域网房间"), Results.Num()), false);
		}
		else
		{
			ShowTips(FString(L"未搜索到任何房间！请确认房主已创建房间且在同一局域网"), true);
			ListView_RoomInfo->ClearListItems();
		}
	}
	else
	{
		ShowTips(FString(L"搜索房间失败！请检查网络或重试"), true);
		ListView_RoomInfo->ClearListItems();
	}
}

void UPlayerSessionSelect::OnJoinSessionResult(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Type::Success)
	{
		ShowTips(FString(L"加入房间成功！等待房主进入游戏..."), false);
		Button_EnterRoom->SetIsEnabled(false);
	}
	else
	{
		ShowTips(FString(L"加入房间失败！房间可能已满/不存在/网络异常"), true);
	}
}
