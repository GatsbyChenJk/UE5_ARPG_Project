// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionItem.h"

#include "PlayerSessionSelect.h"
#include "ARPGScripts/Gameplay/Character/Lobby/LobbyComponent.h"
#include "ARPGScripts/Gameplay/PlayerController/LobbyController.h"
#include "ARPGScripts/Widgets/Base/WidgetManager.h"
#include "Components/Button.h"
#include "Components/ListView.h"

void USessionItem::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	USessionData* SessionData = Cast<USessionData>(ListItemObject);
	CachedData = SessionData;
	if (!SessionData) return;

	// 赋值文本框
	if (Text_RoomName) Text_RoomName->SetText(FText::FromString(SessionData->RoomName));
	if (Text_MapName) Text_MapName->SetText(FText::FromString(SessionData->MapName));
	if (Text_MemberInfo) Text_MemberInfo->SetText(FText::FromString(SessionData->MemberInfo));

	Button_Entry->OnClicked.AddDynamic(this,&ThisClass::OnSelected);
	
	if (const auto WidgetSubSystem = GetGameInstance()->GetSubsystem<UWidgetManager>())
	{
		if (UBaseWidget* Parent = WidgetSubSystem->GetOpenedUIByID(FString("1003")))
		{
			ParentWidget = Cast<UPlayerSessionSelect>(Parent);
		}
	}
		
}

void USessionItem::OnSelected()
{
	if (IsValid(ParentWidget))
	{
		ParentWidget->OnSessionItemSelected.Broadcast(CachedData);
	}
}

