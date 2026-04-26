// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Composite/InventoryLeaf.h"

void UInventoryLeaf::ApplyFunction(FuncType Function)
{
	Function(this);
}
