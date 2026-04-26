// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryWidgetUtils.generated.h"

class UWidget;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryWidgetUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(Blueprintable)
	static bool IsWidgetInBounds(const FVector2D& BoundaryPos,const FVector2D& WidgetSize,const FVector2D& MousePos);
	
	UFUNCTION(BlueprintCallable)
	static FVector2D GetWidgetPosition(UWidget* Widget);

	UFUNCTION(BlueprintCallable)
	static FVector2D GetWidgetSize(UWidget* Widget);
	
	static int32 GetIndexFromPosition(const FIntPoint& Position,const int32 Columns);

	static FIntPoint GetPositionFromIndex(const int32 Index,const int32 Columns);

	static FVector2D GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize, const FVector2D& MousePos);
};
