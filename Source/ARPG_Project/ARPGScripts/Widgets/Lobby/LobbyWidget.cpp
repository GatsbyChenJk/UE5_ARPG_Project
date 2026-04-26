// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"

#include "ARPGScripts/Gameplay/Character/Lobby/LobbyComponent.h"
#include "ARPGScripts/Gameplay/Character/Selection/CharacterSelectCamera.h"
#include "ARPGScripts/Gameplay/PlayerController/LobbyController.h"

void ULobbyWidget::OnOpenWidget(APlayerController* OwningController)
{
	Super::OnOpenWidget(OwningController);
	
	if (const auto LobbyController = Cast<ALobbyController>(OwningController))
	{
		if (const auto LobbyComp = LobbyController->GetLobbyWidgetComponent())
		{
			auto RawStagePtr = LobbyComp->GetCharacterChooseStage();
			AActor* ChooseStage = Cast<AActor>(LobbyComp->GetCharacterChooseStage());
			if (IsValid(ChooseStage))
			{
				OwningController->SetViewTargetWithBlend(ChooseStage,0.5f);
				RawStagePtr->PreviewCharacterMesh(FString("00"));
			}
		}
	}
}

void ULobbyWidget::OnCloseWidget(APlayerController* OwningController)
{
	Super::OnCloseWidget(OwningController);

	if (APawn* MyPawn = OwningController->GetPawn())
	{
		OwningController->SetViewTargetWithBlend(MyPawn,0.5f);
	}

}