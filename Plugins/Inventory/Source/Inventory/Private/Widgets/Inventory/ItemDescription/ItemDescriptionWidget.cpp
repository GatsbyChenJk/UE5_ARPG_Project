// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/ItemDescription/ItemDescriptionWidget.h"

#include "Components/SizeBox.h"

FVector2D UItemDescriptionWidget::GetBoxSize() const
{
	return SizeBox->GetDesiredSize();
}

// void UItemDescriptionWidget::SetVisibility(ESlateVisibility InVisibility)
// {
// 	for (auto Child : GetChildren())
// 	{
// 		Child->Collapse();
// 	}
// 	Super::SetVisibility(InVisibility);
// }
