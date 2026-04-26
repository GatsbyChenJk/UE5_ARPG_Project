// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGLeaf_MapChoice.h"

#include "MapAndModeSelect.h"
#include "Components/Button.h"

void UARPGLeaf_MapChoice::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(Texture_MapIcon))
	{
		Image_MapIcon->SetBrushFromTexture(Texture_MapIcon);
	}
}

void UARPGLeaf_MapChoice::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_MapChoose->OnClicked.AddDynamic(this,&ThisClass::OnWidgetClicked);
}

void UARPGLeaf_MapChoice::SetParentWidget(UMapAndModeSelect* Parent)
{
	ParentWidget = Parent;
}

void UARPGLeaf_MapChoice::OnWidgetClicked()
{
	if (IsValid(ParentWidget))
	{
		ParentWidget->SetCurrentMapID(MapID);
		ParentWidget->SetPreviewMapIcon(Texture_MapIcon);
		ParentWidget->SetPreviewMapName(FText::FromString(MapName));
	}
}
