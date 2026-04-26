// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Base/BaseWidget.h"
#include "ARPGScripts/Widgets/Base/IInteractMessageWidget.h"
#include "InventoryHUD.generated.h"

class UInfoMessage;
/**
 *
 */
UCLASS()
class INVENTORY_API UInventoryHUD : public UBaseWidget, public IInteractMessageWidget
{
	GENERATED_BODY()
	virtual void NativeOnInitialized() override;
public:
	// implement in blueprint
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void ShowPickUpMessage(const FString& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void HidePickUpMessage();

	virtual void OnCloseWidget(APlayerController* OwningController) override;
	virtual void OnOpenWidget(APlayerController* OwningController) override;

	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInfoMessage> InfoMessage;

	UFUNCTION()
	void OnNoRoom();
};
