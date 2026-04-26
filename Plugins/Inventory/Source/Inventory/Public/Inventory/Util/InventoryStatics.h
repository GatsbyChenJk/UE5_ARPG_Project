// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Widgets/Utils/InventoryWidgetUtils.h"
#include "InventoryStatics.generated.h"

class AAIController;
class UInventoryBaseComp;
class UInventoryHoverItem;
class UInventoryItem;
class UItemComponent;
enum class EInventoryGridType : uint8;
class UInventoryComp;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInventoryStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryComp* GetInventoryComp(const APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryComp* GetAIInventoryComp(const AAIController* AIController);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static EInventoryGridType GetItemCategoryFromItemComp(const UItemComponent* ItemComp);

	template<typename T,typename FuncT>
	static void ForEach2D(TArray<T>& Array,int32 Index,const FIntPoint& Range2D,int32 GridColumns,const FuncT& Function);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static void ItemHovered(APlayerController* PC,UInventoryItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static void ItemUnhovered(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static UInventoryHoverItem* GetHoverItem(APlayerController* PC);

	static UInventoryBaseComp* GetInventoryWidget(APlayerController* PC);

};

template <typename T, typename FuncT>
void UInventoryStatics::ForEach2D(TArray<T>& Array, int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Function)
{
	for (int32 j = 0;j<Range2D.Y;j++)
	{
		for (int32 i = 0;i<Range2D.X;i++)
		{
			// based on hash search
			// GridCoordinates : hash
			// TileIndex : search by hash
			const FIntPoint GridCoordinates = UInventoryWidgetUtils::GetPositionFromIndex(Index,GridColumns) + FIntPoint(i,j);
			const int32 TileIndex = UInventoryWidgetUtils::GetIndexFromPosition(GridCoordinates,GridColumns);
			if (Array.IsValidIndex(TileIndex))
			{
				Function(Array[TileIndex]);
			}
		}
	}
}

