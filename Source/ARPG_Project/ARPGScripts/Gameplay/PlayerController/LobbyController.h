// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyController.generated.h"

class ULobbyWidgetComponent;
class UInGameWidgetCompoent;
class UWidgetComponent;
class UCharacterChooseWidget;
class UInputAction;
class UInputMappingContext;
class ACameraActor;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API ALobbyController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
public:
	UFUNCTION(Server,Reliable)
	void Server_LoadLevelConfigDataFromTable(const FString& MapID);

	UFUNCTION(Server,Reliable)
	void Server_LoadCharacterDataFromTable(const FString& CharacterID);

	UFUNCTION(Server,Reliable)
	void Server_TravelToLevel(const FString& MapID);

	UFUNCTION(Server,Reliable)
	void Server_LoadSavedData();

	void LoadSavedGame();
	ULobbyWidgetComponent* GetLobbyWidgetComponent();

	UFUNCTION(BlueprintCallable)
	void ShowInteractMessage();

	UFUNCTION(BlueprintCallable)
	void HideInteractMessage();

	UFUNCTION()
	void ToggleLobbyWidget();
private:
	UPROPERTY(EditAnywhere, Category ="Lobby |Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditDefaultsOnly,Category = "Lobby | Interact")
	TObjectPtr<UInputAction> LobbyMenuAction;

	UPROPERTY(EditDefaultsOnly,Category = "Lobby | Interact")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly,Category = "Lobby | Interact")
	TObjectPtr<UInputAction> LoadingAction;

	UPROPERTY()
	TWeakObjectPtr<ULobbyWidgetComponent> LobbyComponent;

	UFUNCTION(Server,Reliable)
	void Server_ChangePlayerPawn();

	UFUNCTION(Client,Reliable)
	void Client_OnPawnChanged();

	UFUNCTION()
	void OnInteractStartTravel();

	UFUNCTION()
	void ToggleLoadingWidget();
};
