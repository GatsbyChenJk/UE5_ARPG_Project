// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGLeaf_IconText.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UARPGLeaf_IconText::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(Texture_Icon))
	{
		Image_Icon->SetBrushFromTexture(Texture_Icon);
	}
}

void UARPGLeaf_IconText::SetValueText(float InValue)
{
	Text_Value->SetText(FText::AsNumber(InValue));
}
