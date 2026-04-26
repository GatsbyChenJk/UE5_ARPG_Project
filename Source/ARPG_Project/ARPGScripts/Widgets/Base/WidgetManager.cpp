// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetManager.h"

#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"

void UWidgetManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

UBaseWidget* UWidgetManager::OpenUI(const FString& WidgetID)
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		FUIWidgetConfig WidgetConfig = GI->GetLoadedUIConfigByID(WidgetID);
		if (!WidgetConfig.WidgetID.IsEmpty())
		{
			if (UBaseWidget* NewUI = CreateWidget<UBaseWidget>(GetGameInstance(), WidgetConfig.WidgetClass))
			{
				NewUI->SetWidgetConfig(WidgetConfig);
				NewUI->OpenWidget();
				OpenedUIMap.Add(WidgetID, NewUI);
				return NewUI;
			}
		}
	}
	return nullptr;
}

UBaseWidget* UWidgetManager::OpenUI(const FString& WidgetID, APlayerController* InPC)
{
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		FUIWidgetConfig WidgetConfig = GI->GetLoadedUIConfigByID(WidgetID);
		if (!WidgetConfig.WidgetID.IsEmpty())
		{
			//if (!IsValid(InPC)) return nullptr;
			if (UBaseWidget* NewUI = CreateWidget<UBaseWidget>(InPC, WidgetConfig.WidgetClass))
			{
				NewUI->SetWidgetConfig(WidgetConfig);
				NewUI->OpenWidget();
				OpenedUIMap.Add(WidgetID, NewUI);
				return NewUI;
			}
		}
	}
	return nullptr;
}

void UWidgetManager::CloseUI(const FString& WidgetID)
{
	if (OpenedUIMap.Contains(WidgetID))
	{
		UBaseWidget* UI = OpenedUIMap[WidgetID];
		UI->CloseWidget();
		OpenedUIMap.Remove(WidgetID);
	}
}

void UWidgetManager::CloseAllUI()
{
	for (auto& Pair : OpenedUIMap)
	{
		Pair.Value->CloseWidget();
	}
	OpenedUIMap.Empty();
}

UBaseWidget* UWidgetManager::GetOpenedUIByID(const FString& WidgetID)
{
	return OpenedUIMap.Contains(WidgetID) ? OpenedUIMap[WidgetID] : nullptr;
}

void UWidgetManager::RegisterWidget(const FString& WidgetID,UBaseWidget* Widget)
{
	if (WidgetID.IsEmpty() || !IsValid(Widget)) return;
	OpenedUIMap.Add(WidgetID, Widget);
	OnWidgetRegistered.Broadcast();
}
