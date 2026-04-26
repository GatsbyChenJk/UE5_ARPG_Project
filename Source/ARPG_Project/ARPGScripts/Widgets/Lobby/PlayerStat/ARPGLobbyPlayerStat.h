// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGComposite.h"
#include "ARPGLobbyPlayerStat.generated.h"

class UButton;
class UTextBlock;
class UARPGSaveGame;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLobbyPlayerStat : public UARPGComposite
{
	GENERATED_BODY()
	void LoadCharacterMoneyData();
	virtual void NativeOnInitialized() override;
public:
	void SetShopItemDescriptionText(FText Desc);

	void SetCurrentAttackPowerUp(float Value);
	void SetCurrentDefensePowerUp(float Value);
	void SetCurrentHealthPowerUp(float Value);
	void SetCurrentMoneyToPay(float Value);
	void SetMoneyValueText(float MoneyVal);

private:
	UFUNCTION()
	void OnCharacterDataLoadComplete(bool bSuccess,int32 SlotIndex,UARPGSaveGame* SaveGame);

	UFUNCTION()
	void OnCharacterPurchase();
	
	UPROPERTY(EditAnywhere,Category="ARPG UI")
	FGameplayTag MoneyTag;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemDescription;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_ItemPrice;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Purchase;

	UPROPERTY()
	float CurrentMoney;
	
	UPROPERTY()
	float CurrentShopItemMoney;

	UPROPERTY()
	FString CurrentPurchaseItemID;

	UPROPERTY()
	float CurrentItemAttackPower;

	UPROPERTY()
	float CurrentItemDefensePower;

	UPROPERTY()
	float CurrentItemHealthMod;
};
