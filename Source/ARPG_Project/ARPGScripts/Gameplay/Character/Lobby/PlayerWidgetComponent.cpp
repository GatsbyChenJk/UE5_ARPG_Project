// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerWidgetComponent.h"

#include "ARPGScripts/Widgets/Base/WidgetManager.h"


UPlayerWidgetComponent::UPlayerWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningController = Cast<APlayerController>(GetOwner());
}

void UPlayerWidgetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

UBaseWidget* UPlayerWidgetComponent::GetLoadingWidget()
{
	return LoadingWidget;
}

void UPlayerWidgetComponent::ConstructAllWidgets()
{
	LoadingWidget = ConstructWidget("1008");
}

UBaseWidget* UPlayerWidgetComponent::ConstructWidget(const FString& WidgetID)
{
	UWidgetManager* WidgetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWidgetManager>();
	if (!WidgetSubsystem)
	{
		return nullptr;
	}
	
	UBaseWidget* OutWidget = WidgetSubsystem->OpenUI(WidgetID);
	OutWidget->CloseWidget();
	return OutWidget;
}

void UPlayerWidgetComponent::ToggleWidget(const FString& WidgetID)
{
	if (const auto WidgetSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UWidgetManager>())
	{
		UBaseWidget* TargetWidget = WidgetSubsystem->GetOpenedUIByID(WidgetID);
		if (!IsValid(TargetWidget)) return;
		TargetWidget->ToggleWidget(OwningController.Get());
	}
}

