// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Character/Lobby/PlayerWidgetComponent.h"
#include "Components/ActorComponent.h"
#include "InGameWidgetCompoent.generated.h"


class UBaseWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UInGameWidgetCompoent : public UPlayerWidgetComponent
{
	GENERATED_BODY()

public:
	UInGameWidgetCompoent();

	// UFUNCTION()
	// void ToggleInGameWidget(const FString& InGameWidgetID);
	
	UFUNCTION()
	UBaseWidget* GetInventoryMenu();

	UFUNCTION()
	UBaseWidget* GetCharacterHUD();

	UFUNCTION()
	UBaseWidget* GetTargetLockWidget();

	UFUNCTION()
	UBaseWidget* GetInteractWidget();
protected:
	virtual void BeginPlay() override;

	virtual void ConstructAllWidgets() override;
private:
	// UPROPERTY()
	// TWeakObjectPtr<APlayerController> OwningController;
	
	UPROPERTY()
	TObjectPtr<UBaseWidget> CharacterHUDWidget;

	UPROPERTY()
	TObjectPtr<UBaseWidget> InventoryMenuWidget;

	UPROPERTY()
	TObjectPtr<UBaseWidget> TargetLockWidget;

	UPROPERTY()
	TObjectPtr<UBaseWidget> InteractWidget;

	// UPROPERTY()
	// TObjectPtr<UBaseWidget> LoadingWidget;

	// UFUNCTION()
	// void InitHUD();
	
};
