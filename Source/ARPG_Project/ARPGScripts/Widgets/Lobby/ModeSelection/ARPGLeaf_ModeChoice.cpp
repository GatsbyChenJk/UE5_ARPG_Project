// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGLeaf_ModeChoice.h"

#include "MapAndModeSelect.h"
#include "Components/Button.h"

void UARPGLeaf_ModeChoice::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_ModeChoose->OnClicked.AddDynamic(this,&ThisClass::OnWidgetClicked);
}

void UARPGLeaf_ModeChoice::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(Text_Mode))
	Text_Mode->SetText(ModeName);
}

void UARPGLeaf_ModeChoice::OnWidgetClicked()
{
	if (IsValid(ParentWidget))
	{
		ParentWidget->SetPreviewModeName(ModeName);
	}
}
