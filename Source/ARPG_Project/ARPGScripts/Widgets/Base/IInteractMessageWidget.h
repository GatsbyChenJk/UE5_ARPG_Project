// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IInteractMessageWidget.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractMessageWidget : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for widgets that can display interact/pickup messages
 */
class ARPG_PROJECT_API IInteractMessageWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void ShowPickUpMessage(const FString& Message);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void HidePickUpMessage();
};
