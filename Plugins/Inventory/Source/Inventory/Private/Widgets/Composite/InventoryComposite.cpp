// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Composite/InventoryComposite.h"

#include "Blueprint/WidgetTree.h"

void UInventoryComposite::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UInventoryCompositeBase* Composite = Cast<UInventoryCompositeBase>(Widget); IsValid(Composite))
		{
			Children.Add(Composite);
			Composite->Collapse();
		}
	});
}

void UInventoryComposite::ApplyFunction(FuncType Function)
{
	for (auto& Child : Children)
	{
		Child->ApplyFunction(Function);
	}
}

void UInventoryComposite::Collapse()
{
	for (auto& Child : Children)
	{
		Child->Collapse();
	}
}
