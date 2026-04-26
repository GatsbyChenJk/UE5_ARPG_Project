// Fill out your copyright notice in the Description page of Project Settings.


#include "AIWidgetComponent.h"

#include "ARPGScripts/Gameplay/Character/InGame/InGameAICharacter.h"
#include "ARPGScripts/Widgets/InGame/HUDWidgets/ARPGLeaf_ProgressBar.h"
#include "Components/WidgetComponent.h"


UAIWidgetComponent::UAIWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UAIWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerAIChar = Cast<AInGameAICharacter>(GetOwner());

	HealthBarWidget = Cast<UARPGLeaf_ProgressBar>(GetUserWidgetObject());
	if (IsValid(HealthBarWidget))
	{
		HealthBarWidget->SetPercent(1.0f);
	}
}

void UAIWidgetComponent::SetHealthPercent(float HealthPercent)
{
	if (IsValid(HealthBarWidget))
	{
		HealthBarWidget->SetPercent(HealthPercent);
	}
}


