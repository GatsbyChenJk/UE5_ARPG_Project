// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGCompositeBase.h"

void UARPGCompositeBase::Collapse()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UARPGCompositeBase::Expand()
{
	SetVisibility(ESlateVisibility::Visible);
}
