// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyComponent.h"
#include "EngineUtils.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Character/Selection/CharacterSelectCamera.h"
#include "ARPGScripts/Widgets/Base/WidgetManager.h"
#include "ARPGScripts/Widgets/Lobby/CharacterSelection/CharacterSelect.h"

ULobbyWidgetComponent::ULobbyWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULobbyWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CharacterChooseStage = Cast<ACharacterSelectCamera>(FindCharacterChooseStage());

	if (!GetOwner()->HasAuthority())
	{
		if (const auto WidgetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWidgetManager>())
		{
			WidgetSubsystem->OnWidgetRegistered.AddDynamic(this, &ThisClass::BindWidgetCallBack);
		}
	}

	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->OnWidgetDataTableLoaded.AddDynamic(this,&ThisClass::ConstructAllWidgets);
		GI->LoadWidgetDataTableAsync();
	}
	
}

void ULobbyWidgetComponent::ConstructAllWidgets()
{
	Super::ConstructAllWidgets();

	LobbyWidget = ConstructWidget("1001");
	InteractWidget = ConstructWidget("1009");
	InteractWidget->OpenWidget();
}


ACameraActor* ULobbyWidgetComponent::FindCharacterChooseStage()
{
	if (!IsValid(CharacterChooseCameraClass))
	{
		return nullptr;
	}
	
	for (TActorIterator<ACameraActor> It(GetWorld());It;++It)
	{
		ACameraActor* CameraActor = *It;
		if (IsValid(CameraActor) && CameraActor->IsA(CharacterChooseCameraClass))
		{
			return CameraActor;
		}
	}
	UE_LOG(LogTemp,Warning,TEXT("no character choose stage in level"));
	return nullptr;
}


ACharacterSelectCamera* ULobbyWidgetComponent::GetCharacterChooseStage()
{
	if (IsValid(CharacterChooseStage))
	{
		return CharacterChooseStage;
	}
	else
	{
		return Cast<ACharacterSelectCamera>(FindCharacterChooseStage());
	}
}

UBaseWidget* ULobbyWidgetComponent::GetInteractWidget()
{
	return InteractWidget;
}

void ULobbyWidgetComponent::BindWidgetCallBack()
{
	if (const auto WidgetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWidgetManager>())
	{
		if (UCharacterSelect* CharSelectWidget = Cast<UCharacterSelect>(WidgetSubsystem->GetOpenedUIByID(FString("1002"))))
		{
			if (IsValid(CharacterChooseStage))
			{
				CharSelectWidget->OnCharacterChosen.AddDynamic(CharacterChooseStage,&ACharacterSelectCamera::PreviewCharacterMesh);
			}
		}
	}
}




