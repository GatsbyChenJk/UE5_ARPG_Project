// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Composite/InventoryLeafText.h"

#include "Components/TextBlock.h"

void UInventoryLeafText::SetText(const FText& Text) const
{
	Text_LeafText->SetText(Text);
}

void UInventoryLeafText::NativePreConstruct()
{
	Super::NativePreConstruct();

	FSlateFontInfo FontInfo = Text_LeafText->GetFont();
	FontInfo.Size = FontSize;
    	
	Text_LeafText->SetFont(FontInfo);
}
