// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerWidgetComponent.h"
#include "Camera/CameraActor.h"
#include "LobbyComponent.generated.h"


class UBaseWidget;
class UPlayerSessionSelect;
class ACharacterSelectCamera;
class UCharacterChooseWidget;
class ALobbyController;
class UGameModeSelect;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API ULobbyWidgetComponent : public UPlayerWidgetComponent
{
	GENERATED_BODY()

public:
	ULobbyWidgetComponent();
	
	// void ToggleLobbyWidget(const FString& WidgetID);

	ACharacterSelectCamera* GetCharacterChooseStage();

	UBaseWidget* GetInteractWidget();

	ACameraActor* FindCharacterChooseStage();
protected:
	virtual void BeginPlay() override;

	virtual void ConstructAllWidgets() override;
private:
	// UPROPERTY()
	// TWeakObjectPtr<APlayerController> OwningController;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby | Character")
	TSubclassOf<ACameraActor> CharacterChooseCameraClass;
	
	UPROPERTY()
	TObjectPtr<ACharacterSelectCamera> CharacterChooseStage;

	//reconstruct start

	UPROPERTY()
	TObjectPtr<UBaseWidget> LobbyWidget;

	UPROPERTY()
	TObjectPtr<UBaseWidget> InteractWidget;

	UFUNCTION()
	void BindWidgetCallBack();

	//reconstruct end
};
