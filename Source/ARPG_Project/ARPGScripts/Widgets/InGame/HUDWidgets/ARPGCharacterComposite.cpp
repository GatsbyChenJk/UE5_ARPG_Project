// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGCharacterComposite.h"

void UARPGCharacterComposite::OnOpenWidget(APlayerController* OwningController)
{
	this->SetVisibility(ESlateVisibility::Visible);
	bIsWidgetOpen = true;
}

void UARPGCharacterComposite::OnCloseWidget(APlayerController* OwningController)
{
	this->SetVisibility(ESlateVisibility::Collapsed);
	bIsWidgetOpen = false;
}
