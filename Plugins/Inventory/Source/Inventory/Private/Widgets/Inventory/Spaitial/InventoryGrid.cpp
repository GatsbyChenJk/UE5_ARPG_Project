// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spaitial/InventoryGrid.h"

#include "Inventory.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Inventory/InventoryItem.h"
#include "Inventory/Component/InventoryComp.h"
#include "Inventory/Util/InventoryStatics.h"
#include "Items/Components/ItemComponent.h"
#include "Items/Fragment/ItemFragmentTag.h"
#include "UObject/ObjectMacros.h"
#include "Widgets/Inventory/HoverItems/InventoryHoverItem.h"
#include "Widgets/Inventory/PopUpMenu/ItemPopUp.h"
#include "Widgets/Inventory/Spaitial/InventoryGridSlot.h"
#include "Widgets/Inventory/Spaitial/SpaitialInventoryComp.h"
#include "Widgets/Utils/InventoryWidgetUtils.h"
#include "Widgets/Inventory/Spaitial/SlottedItem/InventorySlottedItem.h"

void UInventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	ConstructGrid();

	InventoryComp = UInventoryStatics::GetInventoryComp(GetOwningPlayer());
	InventoryComp->OnItemAdded.AddDynamic(this,&ThisClass::AddItem);
	InventoryComp->OnStackChange.AddDynamic(this,&ThisClass::AddStacks);

	if (USpaitialInventoryComp* InventoryMenu = Cast<USpaitialInventoryComp>(InventoryComp->GetInventoryMenu()))
	{
		InventoryMenu->OnInventoryMenuToggled.AddDynamic(this,&ThisClass::OnIventoryMenuToggled);
	}
}

void UInventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FVector2D CanvasPosition = UInventoryWidgetUtils::GetWidgetPosition(CanvasPanel);
	const FVector2D MousePositioin = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());

	if (CursorExitedCanvas(CanvasPosition,UInventoryWidgetUtils::GetWidgetSize(CanvasPanel),MousePositioin))
	{
		return;
	}

	UpdateTileParameters(CanvasPosition,MousePositioin);
}

void UInventoryGrid::UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition)
{
	if (!bMouseWithinCanvas) return;
	
	const FIntPoint HoveredTileCoordinate = CalculateHoveredCoordinates(CanvasPosition,MousePosition);

	LastTileParameters = TileParameters;
	TileParameters.TilePositions = HoveredTileCoordinate;
	TileParameters.TileIndex = UInventoryWidgetUtils::GetIndexFromPosition(HoveredTileCoordinate,Column);
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPosition,MousePosition);

	OnTileParameterUpdated(TileParameters);
}

FIntPoint UInventoryGrid::CalculateHoveredCoordinates(const FVector2D& CanvasPosition, const FVector2D& MousePosition) const
{
	return FIntPoint
	{
	static_cast<int32>(FMath::FloorToInt((MousePosition.X - CanvasPosition.X)/TileSize)),
	static_cast<int32>(FMath::FloorToInt((MousePosition.Y - CanvasPosition.Y)/TileSize))
	};
}

EInventoryTileQuadrant UInventoryGrid::CalculateTileQuadrant(const FVector2D& CanvasPosition,
	const FVector2D& MousePosition) const
{
	const int32 TileLocalX = FMath::Fmod(MousePosition.X - CanvasPosition.X,TileSize);
	const int32 TileLocalY = FMath::Fmod(MousePosition.Y - CanvasPosition.Y,TileSize);

	const bool bIsTop = TileLocalY < TileSize/2.0f;
	const bool bIsLeft = TileLocalX < TileSize/2.0f;

	EInventoryTileQuadrant TileQuadrant{EInventoryTileQuadrant::None};
	if (bIsTop && bIsLeft) TileQuadrant = EInventoryTileQuadrant::TopLeft;
	else if (!bIsTop && bIsLeft) TileQuadrant = EInventoryTileQuadrant::BottomLeft;
	else if (bIsTop && !bIsLeft) TileQuadrant = EInventoryTileQuadrant::TopRight;
	else if (!bIsTop && !bIsLeft) TileQuadrant = EInventoryTileQuadrant::BottomRight;

	return TileQuadrant;
}

void UInventoryGrid::OnTileParameterUpdated(const FInventoryTileParameters& InParameters)
{
	if (!IsValid(HoverItem)) return;

	const FIntPoint Dimensions = HoverItem->GetGridDimensions();

	const FIntPoint StartCoordinate = CalculateStartingCoordinates(TileParameters.TilePositions,Dimensions,TileParameters.TileQuadrant);
	ItemDropIndex = UInventoryWidgetUtils::GetIndexFromPosition(StartCoordinate,Column);
	
	CurrentQueryResult = CheckHoverPosition(StartCoordinate,Dimensions);

	if (CurrentQueryResult.bHasSpace)
	{
		HightlightSlots(ItemDropIndex,Dimensions);
		return;
	}
	UnHightlightSlots(LastHightlightIndex,LastHightlightDimensions);

	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		const FGridFragment* GridFragment = GetFragment<FGridFragment>(CurrentQueryResult.ValidItem.Get(),FragmentTag::GridFragment);
		if (!GridFragment) return;

		ChangeHoverType(CurrentQueryResult.UpperLeftIndex,GridFragment->GetGridSize(),EInventoryGridSlotState::GrayedOut);
	}
	
}

FIntPoint UInventoryGrid::CalculateStartingCoordinates(const FIntPoint& Coordinate, const FIntPoint& Dimensions,
	const EInventoryTileQuadrant Quadrant) const
{
	const int32 HasEvenWidth = Dimensions.X % 2 == 0 ? 1 : 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0 ? 1 : 0;

	FIntPoint StartingCoord;
	switch (Quadrant)
	{
	case EInventoryTileQuadrant::TopLeft:
			StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
			StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
	case EInventoryTileQuadrant::TopRight:
			StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
			StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
		break;
	case EInventoryTileQuadrant::BottomLeft:
			StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
			StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	case EInventoryTileQuadrant::BottomRight:
		StartingCoord.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
		StartingCoord.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
		break;
	default:
		UE_LOG(LogInventory, Error, TEXT("Invalid quadrant"));
		return FIntPoint(-1,-1);
	}
	return StartingCoord;
}

FInventorySpaceQueryResult UInventoryGrid::CheckHoverPosition(const FIntPoint& Position,
	const FIntPoint& Dimensions)
{
	FInventorySpaceQueryResult Result;

	if (!IsInGridBounds(UInventoryWidgetUtils::GetIndexFromPosition(Position,Column),Dimensions)) return Result;
	Result.bHasSpace = true;

	TSet<int32> OccupiedUpperLeftIndices;
	UInventoryStatics::ForEach2D(GridSlots,UInventoryWidgetUtils::GetIndexFromPosition(Position,Column),Dimensions,Column,[&](UInventoryGridSlot* GridSlot)
	{
		if (IsValid(GridSlot->GetInventoryItem()))
		{
			OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());
			Result.bHasSpace = false;
		}
	});

	if (OccupiedUpperLeftIndices.Num() == 1)
	{
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator();
		Result.ValidItem = GridSlots[Index]->GetInventoryItem();
		Result.UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
		
	}

	return Result;
}

bool UInventoryGrid::CursorExitedCanvas(const FVector2D& BoundaryPos, const FVector2D& BoundarySize,
	const FVector2D& Location)
{
	bLastMouseWithinCanvas = bMouseWithinCanvas;
	bMouseWithinCanvas = UInventoryWidgetUtils::IsWidgetInBounds(BoundaryPos,BoundarySize,Location);

	if (!bMouseWithinCanvas && bLastMouseWithinCanvas)
	{
		// unhighlight
		UnHightlightSlots(LastHightlightIndex, LastHightlightDimensions);
		return true;
	}
	return false;
}

void UInventoryGrid::HightlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	if (!bMouseWithinCanvas) return;
	UnHightlightSlots(LastHightlightIndex,LastHightlightDimensions);
	UInventoryStatics::ForEach2D(GridSlots,Index,Dimensions,Column,[&](UInventoryGridSlot* GridSlot)
	{
		GridSlot->SetOccupiedTexture();
	});
	LastHightlightDimensions = Dimensions;
	LastHightlightIndex = Index;
}

void UInventoryGrid::UnHightlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	UInventoryStatics::ForEach2D(GridSlots,Index,Dimensions,Column,[&](UInventoryGridSlot* GridSlot)
	{
		if (GridSlot->GetAvailable())
		{
			GridSlot->SetUnoccupiedTexture();
		}
		else
		{
			GridSlot->SetOccupiedTexture();
		}
	});
}

void UInventoryGrid::ChangeHoverType(const int32 Index, const FIntPoint& Dimensions, EInventoryGridSlotState SlotState)
{
	UnHightlightSlots(LastHightlightIndex, LastHightlightDimensions);
	UInventoryStatics::ForEach2D(GridSlots,Index,Dimensions,Column,[State = SlotState](UInventoryGridSlot* GridSlot)
	{
		switch (State)
		{
		case EInventoryGridSlotState::Occupied:
			GridSlot->SetOccupiedTexture();
			break;
		case EInventoryGridSlotState::Unoccupied:
			GridSlot->SetUnoccupiedTexture();
			break;
		case EInventoryGridSlotState::GrayedOut:
			GridSlot->SetGrayedOutTexture();
			break;
		case EInventoryGridSlotState::Selected:
			GridSlot->SetSelectedTexture();
			break;
		}
	});
}

void UInventoryGrid::PutItemAtIndex(const int32 Index)
{
	AddItemAtIndex(HoverItem->GetInventoryItem(),Index,HoverItem->GetIsStackable(),HoverItem->GetStackCount());
	UpdateGridSlot(HoverItem->GetInventoryItem(),Index,HoverItem->GetIsStackable(),HoverItem->GetStackCount());
	ClearHoverItem();
}

void UInventoryGrid::ClearHoverItem()
{
	if (!IsValid(HoverItem)) return;

	HoverItem->SetInventoryItem(nullptr);
	HoverItem->SetIsStackable(false);
	HoverItem->SetPreviousGridIndex(INDEX_NONE);
	HoverItem->UpdateStackCount(0);
	HoverItem->SetImageBrush(FSlateNoResource());

	HoverItem->RemoveFromParent();
	HoverItem = nullptr;

	// set mouse cursor
	ShowCursor();
}

bool UInventoryGrid::IsSameStackable(const UInventoryItem* ClickedItem) const
{
	const bool bIsSameItem = ClickedItem == HoverItem->GetInventoryItem();
	const bool bIsStackable = ClickedItem->IsStackable();
	return bIsSameItem && bIsStackable && HoverItem->GetItemTag().MatchesTagExact(ClickedItem->GetItemManifest().GetGameplayTag());
}

void UInventoryGrid::SwapWithHoverItem(UInventoryItem* ClickedItem, const int32 GridIndex)
{
	if (!IsValid(HoverItem)) return;

	UInventoryItem* TempItem = HoverItem->GetInventoryItem();
    const int32 TempStackCount = HoverItem->GetStackCount();
	const bool TempIsStackable = HoverItem->GetIsStackable();

	AssignHoverItem(ClickedItem,GridIndex,HoverItem->GetPreviousGridIndex());
	RemoveItemFromGrid(ClickedItem,GridIndex);
	AddItemAtIndex(TempItem,ItemDropIndex,TempIsStackable,TempStackCount);
	UpdateGridSlot(TempItem,ItemDropIndex,TempIsStackable,TempStackCount);
}

bool UInventoryGrid::ShouldSwapStackCount(const int32 RoomInClickedSlot, const int32 HoveredStackCount,
	const int32 MaxStackSize) const
{
	return RoomInClickedSlot == 0 && HoveredStackCount < MaxStackSize;
}

void UInventoryGrid::SwapStackCount(const int32 ClickedStackCount, const int32 HoveredStackCount, const int32 Index)
{
	UInventoryGridSlot* GridSlot = GridSlots[Index];
	GridSlot->SetStackCount(HoveredStackCount);

	UInventorySlottedItem* CheckedSlottedItem = SlottedItems.FindChecked(Index);
	CheckedSlottedItem->UpdateStackCount(HoveredStackCount);

	HoverItem->UpdateStackCount(ClickedStackCount);
}

void UInventoryGrid::ConsumeHoverItemStacks(const int32 ClickedStackCount, const int32 HoveredStackCount,
	const int32 Index)
{
	const int32 AmountToTransfer = HoveredStackCount;
	const int32 NewClickedStackCount = ClickedStackCount + AmountToTransfer;

	GridSlots[Index]->SetStackCount(NewClickedStackCount);
	SlottedItems.FindChecked(Index)->UpdateStackCount(NewClickedStackCount);
	ClearHoverItem();
	ShowCursor();

	const FGridFragment* GridFragment = GridSlots[Index]->GetInventoryItem()->GetItemManifest().GetFragmentType<FGridFragment>();
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1,1);
	HightlightSlots(Index,Dimensions);
}

void UInventoryGrid::FillInStack(const int32 FillAmount, const int32 Remainder, const int32 Index)
{
	UInventoryGridSlot* GridSlot = GridSlots[Index];
	const int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;

	GridSlot->SetStackCount(NewStackCount);

	UInventorySlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(Index);
	ClickedSlottedItem->UpdateStackCount(NewStackCount);

	HoverItem->UpdateStackCount(Remainder);
}

void UInventoryGrid::PutHoverItemBack()
{
	if (!IsValid(HoverItem)) return;

	FInventorySlotAvailabilityResult Result = HasRoomForItem(HoverItem->GetInventoryItem(), HoverItem->GetStackCount());
	Result.Item = HoverItem->GetInventoryItem();

	AddStacks(Result);
	ClearHoverItem();
}

void UInventoryGrid::OnIventoryMenuToggled(bool bOpen)
{
	if (!bOpen)
	{
		PutHoverItemBack();
	}
}

void UInventoryGrid::CreatePopupMenu(int32 GridIndex)
{
	UInventoryItem* RightClickedItem = GridSlots[GridIndex]->GetInventoryItem();
	if (!IsValid(RightClickedItem)) return;
	if (IsValid(GridSlots[GridIndex]->GetItemPopup())) return;

	PopUpMenuWidget = CreateWidget<UItemPopUp>(this,PopUpWidgetClass);
	GridSlots[GridIndex]->SetItemPopup(PopUpMenuWidget);
	
	OwningCanvasPanel->AddChild(PopUpMenuWidget);
	if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(PopUpMenuWidget))
	{
		const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
		CanvasSlot->SetPosition(MousePosition - ItemPopUpOffset);
		CanvasSlot->SetSize(PopUpMenuWidget->GetBoxSize());
		CanvasSlot->SetZOrder(CanvasSlot->GetZOrder()+10); // avoid menu create under inventoryMenu
		//GEngine->AddOnScreenDebugMessage(-1,3.0f,FColor::Red,FString::Printf(TEXT("Menu zOrder:%d"), CanvasSlot->GetZOrder()));
	}

	const int32 SliderMax = GridSlots[GridIndex]->GetStackCount() - 1;
	if (RightClickedItem->IsStackable() && SliderMax > 0)
	{
		PopUpMenuWidget->OnSplit.BindDynamic(this, &ThisClass::OnPopUpMenuSplit);
		PopUpMenuWidget->SetSliderParams(SliderMax, FMath::Max(1, GridSlots[GridIndex]->GetStackCount() / 2));
	}
	else
	{
		PopUpMenuWidget->CollapseSplitButton();
	}

	PopUpMenuWidget->OnDrop.BindDynamic(this, &ThisClass::OnPopUpMenuDrop);

	if (RightClickedItem->IsConsumable())
	{
		PopUpMenuWidget->OnConsume.BindDynamic(this, &ThisClass::OnPopUpMenuConsume);
	}
	else
	{
		PopUpMenuWidget->CollapseConsumeButton();
	}

	if (RightClickedItem->IsSellable())
	{
		PopUpMenuWidget->OnSell.BindDynamic(this, &ThisClass::OnPopUpMenuSell);
	}
	else
	{
		PopUpMenuWidget->CollapseSellButton();
	}
}

void UInventoryGrid::DropItem()
{
	if (!IsValid(HoverItem)) return;
	if (!IsValid(HoverItem->GetInventoryItem())) return;

	InventoryComp->Server_DropItem(HoverItem->GetInventoryItem(), HoverItem->GetStackCount());

	ClearHoverItem();
	ShowCursor();
}

bool UInventoryGrid::HasHoverItem() const
{
	return IsValid(HoverItem);
}

UInventoryHoverItem* UInventoryGrid::GetHoverItem()
{
	return HoverItem;
}

UUserWidget* UInventoryGrid::GetVisibleCursorWidget()
{
	if (!IsValid(GetOwningPlayer())) return nullptr;
	if (!IsValid(VisibleCursorWidget))
	{
		VisibleCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(),VisibleCursorWidgetClass);
	}
	return VisibleCursorWidget;
}

UUserWidget* UInventoryGrid::GetHiddenCursorWidget()
{
	if (!IsValid(GetOwningPlayer())) return nullptr;
	if (!IsValid(HiddenCursorWidget))
	{
		HiddenCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(),HiddenCursorWidgetClass);
	}
	return HiddenCursorWidget;
}

void UInventoryGrid::ShowCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default,GetVisibleCursorWidget());
}

void UInventoryGrid::HideCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default,GetHiddenCursorWidget());
}

FInventorySlotAvailabilityResult UInventoryGrid::HasRoomForItem(const UItemComponent* ItemComp)
{
	return HasRoomForItem(ItemComp->GetItemManifest());
}

FInventorySlotAvailabilityResult UInventoryGrid::HasRoomForItem(const UInventoryItem* Item,const int32 StackAmountOverride)
{
	return HasRoomForItem(Item->GetItemManifest(),StackAmountOverride);
}

FIntPoint UInventoryGrid::GetGridDimensions(const FItemManifest& ItemManifest) const
{
	const FGridFragment* GridFragment = ItemManifest.GetFragmentType<FGridFragment>();
	return GridFragment ? GridFragment->GetGridSize() : FIntPoint(1,1);
}

FInventorySlotAvailabilityResult UInventoryGrid::HasRoomForItem(const FItemManifest& ItemManifest,const int32 StackAmountOverride)
{
	FInventorySlotAvailabilityResult Result;

	const FStackableFragment* StackableFragment = ItemManifest.GetFragmentType<FStackableFragment>();
	Result.bStackable = StackableFragment != nullptr;

	int32 AmountToFill = StackableFragment ? StackableFragment->GetStackCount() : 1;
	const int32 MaxStackSize = StackableFragment ? StackableFragment->GetMaxStackSize() : 1;

	if (StackAmountOverride != -1 && Result.bStackable)
	{
		AmountToFill = StackAmountOverride;
	}

	TSet<int32> CheckedIndices;
	
	for (const auto& GridSlot : GridSlots)
	{
		if (AmountToFill == 0) break;

		if (IsIndexClaimed(CheckedIndices,GridSlot->GetTileIndex())) continue;

		if (!IsInGridBounds(GridSlot->GetTileIndex(),GetGridDimensions(ItemManifest))) continue;

		TSet<int32> TentativeIndices;
		if (!HasRoomAtIndex(GridSlot,GetGridDimensions(ItemManifest),CheckedIndices,TentativeIndices,ItemManifest.GetGameplayTag(),MaxStackSize))
		{
			continue;
		}

		const int32 AmountToFillInSlot = DetermineFillAmoundForSlot(Result.bStackable,MaxStackSize,AmountToFill,GridSlot);
		if (AmountToFillInSlot == 0) continue;
		
		CheckedIndices.Append(TentativeIndices);
		
		Result.TotalRoomToFill += AmountToFillInSlot;
		Result.SlotAvailabilities.Emplace(
			FInventorySlotAvailability{
			IsValid(GridSlot->GetInventoryItem()) ? GridSlot->GetUpperLeftIndex() : GridSlot->GetTileIndex(),
			Result.bStackable ? AmountToFillInSlot : 0,
			IsValid(GridSlot->GetInventoryItem())
			});

		AmountToFill -= AmountToFillInSlot;

		Result.Remainder = AmountToFill;

		if (Result.Remainder == 0) return Result;
	}
	
	return Result;
}

void UInventoryGrid::AddStacks(const FInventorySlotAvailabilityResult& Result)
{
	if (!MatchesCategory(Result.Item.Get())) return;

	for (const auto& Availability : Result.SlotAvailabilities)
	{
		if (Availability.bItemAtIndex)
		{
			const auto& GridSlot = GridSlots[Availability.Index];
			const auto& SlottedItem = SlottedItems.FindChecked(Availability.Index);
			SlottedItem->UpdateStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
			GridSlot->SetStackCount(GridSlot->GetStackCount() + Availability.AmountToFill);
		}
		else
		{
			AddItemAtIndex(Result.Item.Get(),Availability.Index,Result.bStackable,Availability.AmountToFill);
			UpdateGridSlot(Result.Item.Get(),Availability.Index,Result.bStackable,Availability.AmountToFill);
		}
	}
}

void UInventoryGrid::OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& InMouseEvent)
{
	check(GridSlots.IsValidIndex(GridIndex));
	UInventoryItem* ClickedItem = GridSlots[GridIndex]->GetInventoryItem();

	if (!IsValid(HoverItem) && IsLeftClick(InMouseEvent))
	{
		PickUp(ClickedItem,GridIndex);
		return;
	}

	if (IsRightClick(InMouseEvent))
	{
		CreatePopupMenu(GridIndex);
		return;
	}

	if (IsSameStackable(ClickedItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FStackableFragment* StackFragment = ClickedItem->GetItemManifest().GetFragmentType<FStackableFragment>();
		const int32 MaxStackSize = StackFragment->GetMaxStackSize();
		const int32 RoomInStackSlot = MaxStackSize - ClickedStackCount;
		const int32 HoveredStackCount = HoverItem->GetStackCount();

		// case 1 : just swap
		if (ShouldSwapStackCount(RoomInStackSlot,HoveredStackCount,MaxStackSize))
		{
			SwapStackCount(ClickedStackCount,HoveredStackCount,GridIndex);
		}

		// case 2 : swap and put into slot
		if (RoomInStackSlot >= HoveredStackCount)
		{
			ConsumeHoverItemStacks(ClickedStackCount,HoveredStackCount,GridIndex);
			return;
		}

		// case 3 : fill to full when hoveritem nubmer is more than item in slot
		if (RoomInStackSlot < HoveredStackCount)
		{
			FillInStack(RoomInStackSlot,HoveredStackCount - RoomInStackSlot,GridIndex);
			return;
		}

		// case 4 : no need to swap
		if (RoomInStackSlot == 0)
		{
			return;
		}
		
	}

	// avoid icon overlap
	if (CurrentQueryResult.ValidItem.IsValid())
	{
		SwapWithHoverItem(ClickedItem,GridIndex);
	}
}

void UInventoryGrid::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& InMouseEvent)
{
	if (!IsValid(HoverItem)) return;
	if (!GridSlots.IsValidIndex(GridIndex)) return;

	if (CurrentQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentQueryResult.UpperLeftIndex))
	{
		OnSlottedItemClicked(CurrentQueryResult.UpperLeftIndex,InMouseEvent);
		return;
	}

	if (!IsInGridBounds(ItemDropIndex,HoverItem->GetGridDimensions())) return;
	auto GridSlot = GridSlots[ItemDropIndex];
	if (!IsValid(GridSlot->GetInventoryItem()))
	{
		// put item in this index
		PutItemAtIndex(ItemDropIndex);
	}
}

void UInventoryGrid::OnGridSlotHovered(int32 GridIndex, const FPointerEvent& InMouseEvent)
{
	if (!IsValid(HoverItem)) return;

	UInventoryGridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->GetAvailable())
	{
		GridSlot->SetOccupiedTexture();
	}
}

void UInventoryGrid::OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& InMouseEvent)
{
	if (!IsValid(HoverItem)) return;

	UInventoryGridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->GetAvailable())
	{
		GridSlot->SetUnoccupiedTexture();
	}
}

void UInventoryGrid::OnPopUpMenuSplit(int SplitAmount, int Index)
{
	UInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem();
	if (!IsValid(RightClickedItem)) return;
	if (!RightClickedItem->IsStackable()) return;

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	UInventoryGridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 StackCount = UpperLeftGridSlot->GetStackCount();
	const int32 NewStackCount = StackCount - SplitAmount;

	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	AssignHoverItem(RightClickedItem, UpperLeftIndex, UpperLeftIndex);
	HoverItem->UpdateStackCount(SplitAmount);
}

void UInventoryGrid::OnPopUpMenuDrop(int Index)
{
	UInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem();
	if (!IsValid(RightClickedItem)) return;

	PickUp(RightClickedItem, Index);
	DropItem();
}

void UInventoryGrid::OnPopUpMenuConsume(int Index)
{
	UInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem();
	if (!IsValid(RightClickedItem)) return;

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	UInventoryGridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 NewStackCount = UpperLeftGridSlot->GetStackCount() - 1;

	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	InventoryComp->Server_ConsumeItem(RightClickedItem);

	if (NewStackCount <= 0)
	{
		RemoveItemFromGrid(RightClickedItem, Index);
	}
}

void UInventoryGrid::OnPopUpMenuSell(int Index)
{
	UInventoryItem* RightClickedItem = GridSlots[Index]->GetInventoryItem();
	if (!IsValid(RightClickedItem)) return;

	const int32 UpperLeftIndex = GridSlots[Index]->GetUpperLeftIndex();
	UInventoryGridSlot* UpperLeftGridSlot = GridSlots[UpperLeftIndex];
	const int32 NewStackCount = UpperLeftGridSlot->GetStackCount() - 1;

	UpperLeftGridSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->UpdateStackCount(NewStackCount);

	InventoryComp->Server_SellItem(RightClickedItem);

	if (NewStackCount <= 0)
	{
		RemoveItemFromGrid(RightClickedItem, Index);
	}
}

void UInventoryGrid::SetOwningCanvasPanel(UCanvasPanel* NewCanvasPanel)
{
	OwningCanvasPanel = NewCanvasPanel;
}


void UInventoryGrid::AddItemToIndices(const FInventorySlotAvailabilityResult& Result, UInventoryItem* Item)
{
	for (const auto& Availability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(Item,Availability.Index,Result.bStackable,Availability.AmountToFill);
		UpdateGridSlot(Item,Availability.Index,Result.bStackable,Availability.AmountToFill);
	}
	
}

void UInventoryGrid::AddItemAtIndex(UInventoryItem* Item, const int32 Index, const bool bStackable,
	const int32 StackCount)
{
	// 1.get fragments (grid and image)
	const FGridFragment* GridFragment = GetFragment<FGridFragment>(Item,FragmentTag::GridFragment);
	const FImageFragment* ImageFragment = GetFragment<FImageFragment>(Item,FragmentTag::ImageFragment);
	if (!GridFragment || !ImageFragment) return;
	// 2.create item ui (widgets)
	UInventorySlottedItem* SlottedItem = CreateSlottedItem(Item,bStackable,StackCount,GridFragment,ImageFragment,Index);
	AddSlottedItemToCanvas(Index,GridFragment,SlottedItem);
	// 3.store ui data in container
	SlottedItems.Add(Index,SlottedItem);
}

UInventorySlottedItem* UInventoryGrid::CreateSlottedItem(UInventoryItem* Item, const bool bStackable,
	const int32 StackAmount, const FGridFragment* GridFragment, const FImageFragment* ImageFragment, const int32 Index) const
{
	UInventorySlottedItem* SlottedItem = CreateWidget<UInventorySlottedItem>(GetOwningPlayer(),SlottedItemClass);
	SlottedItem->SetInventoryItem(Item);
	SetSlateItemImage(GridFragment, ImageFragment, SlottedItem);
	SlottedItem->SetGridIndex(Index);
	SlottedItem->SetIsStackable(bStackable);
	const int32 StackCount = StackAmount > 0 ? StackAmount : 0;
	SlottedItem->UpdateStackCount(StackCount);
	SlottedItem->OnSlottedItemClicked.AddDynamic(this,&ThisClass::OnSlottedItemClicked);

	return SlottedItem;
}

void UInventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FGridFragment* GridFragment,
	UInventorySlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	const FVector2D DrawPos = UInventoryWidgetUtils::GetPositionFromIndex(Index,Column) * TileSize;
	const FVector2D DrawPosWithPadding = DrawPos + FVector2D(GridFragment->GetGridPadding());
	CanvasSlot->SetPosition(DrawPosWithPadding);
}

void UInventoryGrid::UpdateGridSlot(UInventoryItem* Item,const int32 Index,bool bStackableItem,const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));
	const FGridFragment* GridFragment = GetFragment<FGridFragment>(Item,FragmentTag::GridFragment);
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize():FIntPoint(1,1);

	if (bStackableItem)
	{
		GridSlots[Index]->SetStackCount(StackAmount);
	}
	
	UInventoryStatics::ForEach2D(GridSlots,Index,Dimensions,Column,[&](UInventoryGridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(Item);
		GridSlot->SetUpperLeftIndex(Index);
		GridSlot->SetOccupiedTexture();
		GridSlot->SetAvailable(false);
	});
	
}

int32 UInventoryGrid::GetStackAmount(const UInventoryGridSlot* GridSlot) const
{
	int32 CurrentStackCount = GridSlot->GetStackCount();
	if (const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex();UpperLeftIndex != INDEX_NONE)
	{
		UInventoryGridSlot* UpperLeftSlot = GridSlots[UpperLeftIndex];
		CurrentStackCount = UpperLeftSlot->GetStackCount();
	}
    return CurrentStackCount;	
}

int32 UInventoryGrid::DetermineFillAmoundForSlot(const bool bStackable, const int32 MaxStackSize,
	const int32 AmountToFill, const UInventoryGridSlot* GridSlot) const
{
	const int32 RoomInSlot = MaxStackSize - GetStackAmount(GridSlot);
	return bStackable ? FMath::Min(AmountToFill,RoomInSlot) : AmountToFill;
}

bool UInventoryGrid::IsInGridBounds(const int32 StartIndex,const FIntPoint& ItemDimension) const
{
	if (StartIndex < 0 || StartIndex >= GridSlots.Num()) return false;
	int32 EndColumn = (StartIndex % Column) + ItemDimension.X;
	int32 EndRow = (StartIndex / Column) + ItemDimension.Y;
	return EndColumn <= Column && EndRow <= Row;
}

bool UInventoryGrid::DoesItemMatchType(const UInventoryItem* SubItem, const FGameplayTag& ItemType) const
{
	return SubItem->GetItemManifest().GetGameplayTag().MatchesTagExact(ItemType);
}

bool UInventoryGrid::IsUpperLeftIndex(const UInventoryGridSlot* GridSlot,const UInventoryGridSlot* SubGridSlot)
{
	return SubGridSlot->GetUpperLeftIndex() == GridSlot->GetTileIndex();
}

bool UInventoryGrid::CheckSlotConstraints(const UInventoryGridSlot* GridSlot,const UInventoryGridSlot* SubGridSlot,const TSet<int32>& CheckedSet,
	TSet<int32>& OutIndices,const FGameplayTag& ItemType,const int32 MaxStackSize)
{
	if (IsIndexClaimed(CheckedSet,SubGridSlot->GetTileIndex())) return false;

	if (!IsValid(SubGridSlot->GetInventoryItem()))
	{
		OutIndices.Add(SubGridSlot->GetTileIndex());
		return true;
	}

	if (!IsUpperLeftIndex(GridSlot,SubGridSlot)) return false;

	const UInventoryItem* SubItem = SubGridSlot->GetInventoryItem();
	if (!SubItem->IsStackable()) return false;

	if (!DoesItemMatchType(SubItem,ItemType)) return false;

	if (GridSlot->GetStackCount() >= MaxStackSize) return false;
	
	return true;
}

bool UInventoryGrid::HasRoomAtIndex(const UInventoryGridSlot* GridSlot, const FIntPoint GridDimensions,const TSet<int32>& CheckedIndices,
	TSet<int32>& OutIndices,const FGameplayTag& ItemType,const int32 MaxStackSize)
{
	bool bHasRoomAtIndex = true;

	UInventoryStatics::ForEach2D(GridSlots,GridSlot->GetTileIndex(),GridDimensions,Column,[&](UInventoryGridSlot* SubGridSlot)
	{
		if (CheckSlotConstraints(GridSlot,SubGridSlot,CheckedIndices,OutIndices,ItemType,MaxStackSize))
		{
			OutIndices.Add(SubGridSlot->GetTileIndex());
		}
		else
		{
			bHasRoomAtIndex = false;
		}
	});
	
	return bHasRoomAtIndex;
}

bool UInventoryGrid::IsRightClick(const FPointerEvent& Event) const
{
	return Event.GetEffectingButton() == EKeys::RightMouseButton;
}

bool UInventoryGrid::IsLeftClick(const FPointerEvent& Event) const
{
	return Event.GetEffectingButton() == EKeys::LeftMouseButton;
}

void UInventoryGrid::PickUp(UInventoryItem* InventoryItem, const int32 GridIndex)
{
	AssignHoverItem(InventoryItem,GridIndex,GridIndex);
	RemoveItemFromGrid(InventoryItem,GridIndex);
}

void UInventoryGrid::AssignHoverItem(UInventoryItem* InventoryItem)
{
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UInventoryHoverItem>(GetOwningPlayer(),HoverItemClass);
	}

	const FGridFragment* GridFragment = GetFragment<FGridFragment>(InventoryItem,FragmentTag::GridFragment);
	const FImageFragment* ImageFragment = GetFragment<FImageFragment>(InventoryItem,FragmentTag::ImageFragment);
	if (!GridFragment || !ImageFragment) return;

	const FVector2D DrawSize = GetDrawSize(GridFragment);

	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = DrawSize * UWidgetLayoutLibrary::GetViewportScale(this);

	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetInventoryItem(InventoryItem);
	HoverItem->SetIsStackable(InventoryItem->IsStackable());

	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default,HoverItem);
}

void UInventoryGrid::OnHide()
{
	PutHoverItemBack();
}

void UInventoryGrid::AssignHoverItem(UInventoryItem* InventoryItem, const int32 GridIndex, const int32 PreviousIndex)
{
	AssignHoverItem(InventoryItem);

	HoverItem->SetPreviousGridIndex(PreviousIndex);
	HoverItem->UpdateStackCount(InventoryItem->IsStackable() ? GridSlots[GridIndex]->GetStackCount() : 0);
}

void UInventoryGrid::RemoveItemFromGrid(UInventoryItem* InventoryItem, const int32 GridIndex)
{
	const FGridFragment* GridFragment = GetFragment<FGridFragment>(InventoryItem,FragmentTag::GridFragment);
	if (!GridFragment) return;

	UInventoryStatics::ForEach2D(GridSlots,GridIndex,GridFragment->GetGridSize(),Column,[&](UInventoryGridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(nullptr);
		GridSlot->SetUpperLeftIndex(INDEX_NONE);
		GridSlot->SetUnoccupiedTexture();
		GridSlot->SetAvailable(true);
		GridSlot->SetStackCount(0);
	});

	if (SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInventorySlottedItem> FoundItem;
		SlottedItems.RemoveAndCopyValue(GridIndex,FoundItem);
		FoundItem->RemoveFromParent();
	}
}

void UInventoryGrid::SetSlateItemImage(const FGridFragment* GridFragment, const FImageFragment* ImageFragment, const UInventorySlottedItem* SlottedItem) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GridFragment);
	SlottedItem->SetImageBrush(Brush);
}

FVector2D UInventoryGrid::GetDrawSize(const FGridFragment* GridFragment) const
{
	const float ItemTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	FVector2D IconSize = GridFragment->GetGridSize() * ItemTileWidth;
	return IconSize;
}

void UInventoryGrid::AddItem(UInventoryItem* Item)
{
	if (!MatchesCategory(Item)) return;

	FInventorySlotAvailabilityResult Result = HasRoomForItem(Item);

	AddItemToIndices(Result,Item);
}

void UInventoryGrid::ConstructGrid()
{
	GridSlots.Reserve(Row * Column);

	for (int j = 0; j < Row; j++)
	{
		for (int i = 0; i < Column; i++)
		{
			UInventoryGridSlot* GridSlot = CreateWidget<UInventoryGridSlot>(this,GridSlotClass);
			CanvasPanel->AddChild(GridSlot);

			const FIntPoint TilePosition(i,j);
			GridSlot->SetTileIndex(UInventoryWidgetUtils::GetIndexFromPosition(TilePosition,Column));

			UCanvasPanelSlot* CanvasCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			CanvasCPS->SetSize(FVector2D(TileSize));
			CanvasCPS->SetPosition(TilePosition * TileSize);

			GridSlots.Add(GridSlot);
			GridSlot->GridSlotClicked.AddDynamic(this,&ThisClass::OnGridSlotClicked);
			GridSlot->GridSlotHovered.AddDynamic(this,&ThisClass::OnGridSlotHovered);
			GridSlot->GridSlotUnhovered.AddDynamic(this,&ThisClass::OnGridSlotUnhovered);
		}
	}
}

bool UInventoryGrid::MatchesCategory(const UInventoryItem* Item) const
{
	return Item->GetItemManifest().GetItemCategory() == GridType;
}
