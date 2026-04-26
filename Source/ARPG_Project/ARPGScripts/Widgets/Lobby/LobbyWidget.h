// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Base/BaseWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API ULobbyWidget : public UBaseWidget
{
	GENERATED_BODY()

public:
	virtual void OnOpenWidget(APlayerController* OwningController) override;

	virtual void OnCloseWidget(APlayerController* OwningController) override;
};
