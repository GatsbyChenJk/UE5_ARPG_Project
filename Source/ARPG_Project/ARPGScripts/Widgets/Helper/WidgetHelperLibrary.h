// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WidgetHelperLibrary.generated.h"

class UListView;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UWidgetHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static int32 GetListEntryIndex(UObject* TargetItem,UListView* TargetListView);
};
