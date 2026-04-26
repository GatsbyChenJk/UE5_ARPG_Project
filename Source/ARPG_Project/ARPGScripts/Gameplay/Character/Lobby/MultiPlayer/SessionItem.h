// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerSessionSelect.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "SessionItem.generated.h"

class UPlayerSessionSelect;
class UButton;
/**
 * 
 */

UCLASS()
class ARPG_PROJECT_API USessionItem : public UUserWidget,public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	void SetRoomName(const FString& RoomName) { Text_RoomName->SetText(FText::FromString(RoomName)); };
	void SetMapName(const FString& MapName){ Text_MapName->SetText(FText::FromString(MapName));};
	void SetMemberInfo(const FString& MemberInfo){Text_MemberInfo->SetText(FText::FromString(MemberInfo));};
		
	UFUNCTION()
	void OnSelected();

private:
	UPROPERTY()
	TObjectPtr<UPlayerSessionSelect> ParentWidget;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Entry;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_RoomName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_MapName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_MemberInfo;

	UPROPERTY()
	TObjectPtr<USessionData> CachedData;
	
};
