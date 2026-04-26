// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Base/BaseWidget.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "PlayerSessionSelect.generated.h"

class UUserObjectListEntry;
class USessionItem;
class UListView;
class UTextBlock;
class UPlayerSessionManager;
class UButton;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionItemSelected,UObject*, Item);

UCLASS()
class ARPG_PROJECT_API USessionData : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString MapName;

	UPROPERTY()
	FString RoomName;

	UPROPERTY()
	FString MemberInfo;
	
};

UCLASS()
class ARPG_PROJECT_API UPlayerSessionSelect : public UBaseWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeOnInitialized() override;

	//TODO: implement widget logic in blueprint
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void OnCreateRoomBtnClicked(const FString& RoomName); // 创建房间按钮

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void OnSearchRoomBtnClicked(); // 搜索房间按钮

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void OnJoinRoomBtnClicked(int32 SelectedIndex); // 加入房间按钮（选中列表索引）

	UFUNCTION()
	void OnSessionItemSelect(UObject* Item);
	
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void SetCurrentSelectedIndex(int32 index) {CurrentSelectedSession = index;};

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	int32 GetCurrentSelectedIndex() {return CurrentSelectedSession;};

	FOnSessionItemSelected OnSessionItemSelected;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite, Category = "Multiplayer")
	TSubclassOf<USessionItem> EntryWidgetClass;

	UPROPERTY(meta=(BindWidget),BlueprintReadWrite)
	TObjectPtr<UListView> ListView_RoomInfo;
private:
	// 存储委托绑定的句柄，避免销毁UI时委托悬空
	FOnCreateSessionCompleteDelegate CreateSessionDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionDelegateHandle;
	
	void ShowTips(const FString& Tip, bool bIsError);
	
	void UpdateRoomList();
	
	void OnCreateSessionResult(FName SessionName,bool bSuccess);
	
	void OnFindSessionsResult(bool bSuccess);

	void OnJoinSessionResult(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	int32 CurrentSelectedSession{INDEX_NONE};
	
	UPlayerSessionManager* SessionMgr = nullptr;

	TArray<FOnlineSessionSearchResult> CachedSearchResults;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Refresh;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_CreateRoom;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_EnterRoom;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Tips;
};
