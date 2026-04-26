// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGComposite.h"
#include "CharacterSelect.generated.h"

class UButton;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyCharacterChosen,const FString&,CharacterID);

UCLASS()
class ARPG_PROJECT_API UCharacterSelect : public UARPGComposite
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
public:
	FOnLobbyCharacterChosen OnCharacterChosen;

	void SetCurrentCharacterID(FString CharacterID) { CurrentCharacterID = CharacterID; };
	
private:
	UFUNCTION()
	void OnConfirmCurrentChoice();

	FString CurrentCharacterID;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Confirm;
};
