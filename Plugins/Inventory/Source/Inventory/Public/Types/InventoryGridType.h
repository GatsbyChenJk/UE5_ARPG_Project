#pragma once

#include "InventoryGridType.generated.h"

class UInventoryItem;

UENUM(BlueprintType)
enum  class EInventoryGridType : uint8
{
	Equippable,
	Consumable,
	Craftable,
	None
};

USTRUCT()
struct FInventorySlotAvailability
{
	GENERATED_BODY()
	FInventorySlotAvailability(){}
	FInventorySlotAvailability(int32 ItemIndex,int32 Room,bool bHasItem):Index(ItemIndex),AmountToFill(Room),bItemAtIndex(bHasItem){}

	int32 Index{INDEX_NONE};
	int32 AmountToFill{0};
	bool bItemAtIndex{false};
};

USTRUCT()
struct FInventorySlotAvailabilityResult
{
	GENERATED_BODY()

	FInventorySlotAvailabilityResult(){}

	TWeakObjectPtr<UInventoryItem> Item;
	int32 TotalRoomToFill{0};
	int32 Remainder{0};
	bool bStackable{false};
	TArray<FInventorySlotAvailability> SlotAvailabilities;
};

UENUM(BlueprintType)
enum class EInventoryTileQuadrant : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
	None
};

USTRUCT(blueprintType)
struct FInventoryTileParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="Inventory")
	FIntPoint TilePositions{};

	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="Inventory")
	int32 TileIndex{INDEX_NONE};

	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="Inventory")
	EInventoryTileQuadrant TileQuadrant{EInventoryTileQuadrant::None};
};

inline bool operator==(const FInventoryTileParameters& A,const FInventoryTileParameters& B)
{
	return A.TilePositions == B.TilePositions
	&& A.TileIndex == B.TileIndex
	&& A.TileQuadrant == B.TileQuadrant;
}

USTRUCT()
struct FInventorySpaceQueryResult
{
	GENERATED_BODY()

	bool bHasSpace{false};

	TWeakObjectPtr<UInventoryItem> ValidItem = nullptr;

	int32 UpperLeftIndex{INDEX_NONE};
	
};