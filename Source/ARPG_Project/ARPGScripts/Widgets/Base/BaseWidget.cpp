// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWidget.h"

void UBaseWidget::OpenWidget()
{
	if (WidgetConfig.WidgetOrder != 0)
	{
		AddToViewport(WidgetConfig.WidgetOrder);
	}
	else
	{
		AddToViewport();
	}
	SetVisibility(ESlateVisibility::Visible);
}

void UBaseWidget::CloseWidget()
{
	this->SetVisibility(ESlateVisibility::Collapsed);
}

void UBaseWidget::OnOpenWidget(APlayerController* OwningController)
{
	this->SetVisibility(ESlateVisibility::Visible);
	bIsWidgetOpen = true;
	
	FInputModeGameAndUI InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(true);
}

void UBaseWidget::OnCloseWidget(APlayerController* OwningController)
{
	this->SetVisibility(ESlateVisibility::Collapsed);
	bIsWidgetOpen = false;
	
	FInputModeGameOnly InputMode;
	OwningController->SetInputMode(InputMode);
	OwningController->SetShowMouseCursor(false);
}

void UBaseWidget::ToggleWidget(APlayerController* OwningController)
{
	if (!IsValid(this) || !IsValid(OwningController)) return;
	
	if (bIsWidgetOpen)
	{
		this->OnCloseWidget(OwningController);
		bIsWidgetOpen = false;
	}
	else
	{
		this->OnOpenWidget(OwningController);
		bIsWidgetOpen = true;
	}
}
