// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetHelperLibrary.h"

#include "Components/ListView.h"

int32 UWidgetHelperLibrary::GetListEntryIndex(UObject* TargetItem, UListView* TargetListView)
{
	if (!IsValid(TargetItem) || !IsValid(TargetListView)) return INDEX_NONE;
	
	int32 OutIndex = TargetListView->GetIndexForItem(TargetItem);
	
	return OutIndex;	
}
