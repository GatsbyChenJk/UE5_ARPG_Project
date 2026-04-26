// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGComposite.h"

#include "Blueprint/WidgetTree.h"

void UARPGComposite::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UARPGCompositeBase* Composite = Cast<UARPGCompositeBase>(Widget); IsValid(Composite))
		{
			Children.Add(Composite);
			//Composite->Collapse();
		}
	});
}

void UARPGComposite::ApplyFunction(ARPGComPositeFunc Function)
{
	for (auto& Child : Children)
	{
		Child->ApplyFunction(Function);
	}
}

void UARPGComposite::Collapse()
{
	for (auto& Child : Children)
	{
		Child->Collapse();
	}
}

UARPGCompositeBase* UARPGComposite::FindChildByTag(const FGameplayTag& InQueryTag)
{
	auto Child = Children.FindByPredicate([InQueryTag](UARPGCompositeBase* Query)
	{
		return Query->GetQueryTag() == InQueryTag;
	});

	return Child != nullptr ? *Child : nullptr;
}
