// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InfoMessage.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInfoMessage : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent,Category="Inventory")
	void MessageShow();

	UFUNCTION(BlueprintImplementableEvent,Category="Inventory")
	void MessageHide();
	
	void SetMessage(const FText& Msg);
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Message;

	UPROPERTY(EditAnywhere,Category="Inventory")
	float MessageLifeTime{3.0f};

	FTimerHandle MessageTimer;
	bool bIsMessageActive{false};
};
