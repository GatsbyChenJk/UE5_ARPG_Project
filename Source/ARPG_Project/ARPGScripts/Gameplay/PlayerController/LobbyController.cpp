// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Base/GameModes/LobbyGameMode.h"
#include "ARPGScripts/Gameplay/Base/SaveGameSystem/ARPGSaveGameManager.h"
#include "ARPGScripts/Gameplay/Character/Lobby/LobbyComponent.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameWidgetCompoent.h"
#include "ARPGScripts/Gameplay/GameMap/MapLoadingManager.h"
#include "ARPGScripts/Widgets/Base/BaseWidget.h"
#include "ARPGScripts/Widgets/Base/IInteractMessageWidget.h"

void ALobbyController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}

	LobbyComponent = FindComponentByClass<ULobbyWidgetComponent>();
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GI->InitSessionManager(this);
	}
}

void ALobbyController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* InputComp = CastChecked<UEnhancedInputComponent>(InputComponent);
	
	InputComp->BindAction(LobbyMenuAction,ETriggerEvent::Started,this,&ThisClass::ToggleLobbyWidget);
	InputComp->BindAction(InteractAction,ETriggerEvent::Started,this,&ThisClass::OnInteractStartTravel);
	InputComp->BindAction(LoadingAction,ETriggerEvent::Started,this,&ThisClass::ToggleLoadingWidget);
}

void ALobbyController::Server_LoadCharacterDataFromTable_Implementation(const FString& CharacterID)
{
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GameInstance->LoadCharacterDataFromTable(CharacterID);
	}

	Server_ChangePlayerPawn();
}

void ALobbyController::Server_LoadLevelConfigDataFromTable_Implementation(const FString& MapID)
{
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		GameInstance->LoadLevelConfigDataFromTable(MapID);
	}
}

void ALobbyController::Server_ChangePlayerPawn_Implementation()
{
	FTransform SpawnTransform;
	if (APawn* PlayerPawn = GetPawn())
	{
		SpawnTransform = PlayerPawn->GetActorTransform();
		UnPossess();
		PlayerPawn->Destroy();
	}

	UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetWorld()->GetGameInstance());
	TSubclassOf<APawn> PawnClassToSpawn = nullptr;
	if (GameInstance && GameInstance->GetCharacterData().bIsCharacterSet)
	{
		PawnClassToSpawn = GameInstance->GetCharacterData().GetCharacterManifest()->CharacterClass;
	}

	// for corner case : char is null in data table 
	if (PawnClassToSpawn == nullptr)
	{
		// use thorn(thirdperson) character as default pawn
		FCharacterData CharacterData = GameInstance->GetCharacterDataByID(FString("000"));
		PawnClassToSpawn = CharacterData.GetCharacterManifest()->CharacterClass;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnClassToSpawn, SpawnTransform, SpawnParams);
    
	if (NewPawn)
	{
		// possess new pawn
		Possess(NewPawn);

		Client_OnPawnChanged();
		// TODO : 后期完善新内容
	}
	
}

void ALobbyController::Client_OnPawnChanged_Implementation()
{
	APawn* PlayerPawn = GetPawn();
	if (PlayerPawn && IsLocalController())
	{
		SetViewTargetWithBlend(PlayerPawn,0.5f);
	}
	
	ToggleLobbyWidget();
}

void ALobbyController::Server_TravelToLevel_Implementation(const FString& MapID)
{
	if (ALobbyGameMode* GameMode = Cast<ALobbyGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->Server_TravelToLevelByMapID(MapID);
	}
}


void ALobbyController::LoadSavedGame()
{
	if (auto saveMgr = GetGameInstance()->GetSubsystem<UARPGSaveGameManager>())
	{
		saveMgr->AsyncLoadGame(0);
	}
}

void ALobbyController::Server_LoadSavedData_Implementation()
{
	LoadSavedGame();
}

ULobbyWidgetComponent* ALobbyController::GetLobbyWidgetComponent()
{
	return LobbyComponent.Get();
}

void ALobbyController::ShowInteractMessage()
{
	if (LobbyComponent.IsValid())
	{
		if (UBaseWidget* Widget = LobbyComponent->GetInteractWidget())
		{
			if (Widget->Implements<UInteractMessageWidget>())
			{
				IInteractMessageWidget::Execute_ShowPickUpMessage(Widget, LobbyComponent->GetInteractMessage());
			}
		}
		LobbyComponent->SetSceneInteractable(true);
	}
}

void ALobbyController::HideInteractMessage()
{
	if (LobbyComponent.IsValid())
	{
		if (UBaseWidget* Widget = LobbyComponent->GetInteractWidget())
		{
			if (Widget->Implements<UInteractMessageWidget>())
			{
				IInteractMessageWidget::Execute_HidePickUpMessage(Widget);
			}
		}
		LobbyComponent->SetSceneInteractable(false);
	}
}


void ALobbyController::ToggleLobbyWidget()
{
	if (!LobbyComponent.IsValid() || !IsLocalController()) return;
	LobbyComponent->ToggleWidget(FString("1001"));
}



void ALobbyController::OnInteractStartTravel()
{
	if (!LobbyComponent.Get()->GetSceneInteractable()) return;
	FString MapID = FString("M03");
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetWorld()->GetGameInstance()))
	{
		auto MapManifest = GameInstance->GetMapManifest();
		if (!MapManifest.MapID.IsEmpty())
		{
			MapID = MapManifest.MapID;
		}
	}
	Server_TravelToLevel(MapID);
	ToggleLoadingWidget();
}

void ALobbyController::ToggleLoadingWidget()
{
	if (!LobbyComponent.IsValid() || !IsLocalController()) return;
	LobbyComponent->ToggleWidget(FString("1008"));
}
