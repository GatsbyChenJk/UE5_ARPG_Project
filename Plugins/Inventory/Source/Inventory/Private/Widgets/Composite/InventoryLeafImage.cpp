// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Composite/InventoryLeafImage.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"

void UInventoryLeafImage::SetImage(UTexture2D* Texture) const
{
	Image_Icon->SetBrushFromTexture(Texture);
}

void UInventoryLeafImage::SetBoxSize(const FVector2D& Size) const
{
	SizeBox_Icon->SetWidthOverride(Size.X);
	SizeBox_Icon->SetHeightOverride(Size.Y);
}

void UInventoryLeafImage::SetImageSize(const FVector2D& Size) const
{
	Image_Icon->SetDesiredSizeOverride(Size);
}

FVector2D UInventoryLeafImage::GetImageSize() const
{
	return Image_Icon->GetDesiredSize();
}
