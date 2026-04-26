// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spaitial/SpaitialInventoryComp.h"
#include "Inventory.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/Inventory/Spaitial/InventoryGrid.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Inventory/Component/InventoryComp.h"
#include "Inventory/Util/InventoryStatics.h"
#include "Widgets/Inventory/HoverItems/InventoryHoverItem.h"
#include "Widgets/Inventory/ItemDescription/ItemDescriptionWidget.h"
#include "Widgets/Inventory/Spaitial/InventoryEquipmentSlot.h"
#include "Widgets/Inventory/Spaitial/SlottedItem/InventoryEquipSlottedItem.h"


void USpaitialInventoryComp::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Equippable->OnClicked.AddDynamic(this,&ThisClass::ShowEquippables);
	Button_Consumable->OnClicked.AddDynamic(this,&ThisClass::ShowConsumables);
	Button_Craftable->OnClicked.AddDynamic(this,&ThisClass::ShowCraftables);

	Grid_Equippables->SetOwningCanvasPanel(CanvasPanel);
	Grid_Consumables->SetOwningCanvasPanel(CanvasPanel);
	Grid_Craftables->SetOwningCanvasPanel(CanvasPanel);

	ShowEquippables();

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UInventoryEquipmentSlot* EquippedGridSlot = Cast<UInventoryEquipmentSlot>(Widget);
		if (IsValid(EquippedGridSlot))
		{
			EquippedGridSlots.Add(EquippedGridSlot);
			EquippedGridSlot->EquippedGridSlotClicked.AddDynamic(this, &ThisClass::EquippedGridSlotClicked);
		}
	});
}

void USpaitialInventoryComp::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(ItemDescWidget)) return;
	SetItemDescriptionSizeAndPosition(ItemDescWidget,CanvasPanel);
}

FReply USpaitialInventoryComp::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (IsValid(ActiveGrid.Get()))
	{
		ActiveGrid->DropItem();
	}
	return FReply::Handled();
}

FInventorySlotAvailabilityResult USpaitialInventoryComp::HasRoomForItem(UItemComponent* ItemComp) const
{
	switch (UInventoryStatics::GetItemCategoryFromItemComp(ItemComp))
	{
	case EInventoryGridType::Equippable:
		return Grid_Equippables->HasRoomForItem(ItemComp);
	case EInventoryGridType::Consumable:
		return Grid_Consumables->HasRoomForItem(ItemComp);
	case EInventoryGridType::Craftable:
		return Grid_Craftables->HasRoomForItem(ItemComp);
	default:
		UE_LOG(LogInventory,Error,TEXT("No Result was got in ItemComponent!"))
		return FInventorySlotAvailabilityResult();	
	}
}

UItemDescriptionWidget* USpaitialInventoryComp::GetItemDescription()
{
	if (!IsValid(ItemDescWidget))
	{
		ItemDescWidget = CreateWidget<UItemDescriptionWidget>(GetOwningPlayer(),ItemDescWidgetClass);
		CanvasPanel->AddChild(ItemDescWidget);
	}
	return ItemDescWidget;
}

void USpaitialInventoryComp::OnItemHovered(UInventoryItem* Item)
{
	const auto& Manifest = Item->GetItemManifest();
	UItemDescriptionWidget* DescriptionWidget = GetItemDescription();
	DescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);

	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);
	//GetOwningPlayer()->GetWorldTimerManager().ClearTimer(EquippedDescriptionTimer);

	FTimerDelegate DescriptionTimerDelegate;
	DescriptionTimerDelegate.BindLambda([this, Item, &Manifest, DescriptionWidget]()
	{
		GetItemDescription()->SetVisibility(ESlateVisibility::HitTestInvisible);
		Manifest.AssimilateInventoryFragments(DescriptionWidget);
		
		// For the second item description, showing the equipped item of this type.
		//FTimerDelegate EquippedDescriptionTimerDelegate;
		//EquippedDescriptionTimerDelegate.BindUObject(this, &ThisClass::ShowEquippedItemDescription, Item);
		//GetOwningPlayer()->GetWorldTimerManager().SetTimer(EquippedDescriptionTimer, EquippedDescriptionTimerDelegate, EquippedDescriptionTimerDelay, false);
	});

	GetOwningPlayer()->GetWorldTimerManager().SetTimer(DescriptionTimer, DescriptionTimerDelegate, DescriptionTimerDelay, false);
}

void USpaitialInventoryComp::OnItemUnHovered()
{
	GetItemDescription()->SetVisibility(ESlateVisibility::Collapsed);
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);
	//GetEquippedItemDescription()->SetVisibility(ESlateVisibility::Collapsed);
	//GetOwningPlayer()->GetWorldTimerManager().ClearTimer(EquippedDescriptionTimer);
}

bool USpaitialInventoryComp::HasHoverItem() const
{
	if (Grid_Equippables->HasHoverItem()) return true;
	if (Grid_Consumables->HasHoverItem()) return true;
	if (Grid_Craftables->HasHoverItem()) return true;
	return false;
}

UInventoryHoverItem* USpaitialInventoryComp::GetHoverItem() const
{
	if (!ActiveGrid.IsValid()) return nullptr;
	return ActiveGrid->GetHoverItem();
}

float USpaitialInventoryComp::GetTileSize() const
{
	return ActiveGrid->GetTileSize();
}

void USpaitialInventoryComp::ToggleWidget(APlayerController* OwningController)
{
	Super::ToggleWidget(OwningController);

	OnInventoryMenuToggled.Broadcast(bIsWidgetOpen);
}

void USpaitialInventoryComp::CharacterEquipAllWeapons()
{
	for (auto WeaponGridSlot : EquippedGridSlots)
	{
		UInventoryItem* Weapon = WeaponGridSlot->GetInventoryItem();

		UInventoryComp* IC = UInventoryStatics::GetInventoryComp(GetOwningPlayer());
		if (IsValid(IC))
		{
			IC->Server_EquipSlotClicked(Weapon,nullptr);
		}
	}
}

void USpaitialInventoryComp::DisableButton(UButton* Button)
{
	Button_Equippable->SetIsEnabled(true);
	Button_Consumable->SetIsEnabled(true);
	Button_Craftable->SetIsEnabled(true);
	Button->SetIsEnabled(false);
}

void USpaitialInventoryComp::SwitchActiveGrid(UInventoryGrid* Grid, UButton* Button)
{
	// TODO ： Create blueprint class for eveny grid
	if (ActiveGrid.IsValid()) 
	{
		ActiveGrid->HideCursor();
		ActiveGrid->OnHide();
	}
	ActiveGrid = Grid;
	if (ActiveGrid.IsValid()) ActiveGrid->ShowCursor();
	DisableButton(Button);
	Switcher->SetActiveWidget(Grid);
}

void USpaitialInventoryComp::ShowEquippables()
{
	SwitchActiveGrid(Grid_Equippables,Button_Equippable);
}

void USpaitialInventoryComp::ShowConsumables()
{
	SwitchActiveGrid(Grid_Consumables,Button_Consumable);
}

void USpaitialInventoryComp::ShowCraftables()
{
	SwitchActiveGrid(Grid_Craftables,Button_Craftable);
}

void USpaitialInventoryComp::EquippedSlottedItemClicked(UInventoryEquipSlottedItem* EquippedSlottedItem)
{
	// Remove the Item Description
	UInventoryStatics::ItemUnhovered(GetOwningPlayer());

	if (IsValid(GetHoverItem()) && GetHoverItem()->GetIsStackable()) return;
	
	// Get Item to Equip
	UInventoryItem* ItemToEquip = IsValid(GetHoverItem()) ? GetHoverItem()->GetInventoryItem() : nullptr;
	
	// Get Item to Unequip
	UInventoryItem* ItemToUnequip = EquippedSlottedItem->GetInventoryItem();

	// Get the Equipped Grid Slot holding this item
	UInventoryEquipmentSlot* EquippedGridSlot = FindSlotWithEquippedItem(ItemToUnequip);

	// Clear the equipped grid slot of this item (set its inventory item to nullptr)
	ClearSlotOfItem(EquippedGridSlot);

	// Assign previously equipped item as the hover item
	Grid_Equippables->AssignHoverItem(ItemToUnequip);
	
	// Remove of the equipped slotted item from the equipped grid slot
	RemoveEquippedSlottedItem(EquippedSlottedItem);

	// Make a new equipped slotted item (for the item we held in HoverItem)
	MakeEquippedSlottedItem(EquippedSlottedItem, EquippedGridSlot, ItemToEquip);
	
	// Broadcast delegates for OnItemEquipped/OnItemUnequipped (from the IC)
	BroadcastSlotClickedDelegates(ItemToEquip, ItemToUnequip);
}

void USpaitialInventoryComp::EquippedGridSlotClicked(UInventoryEquipmentSlot* EquippedGridSlot,
                                                     const FGameplayTag& EquipmentTypeTag)
{
	if (!CanEquipHoverItem(EquippedGridSlot, EquipmentTypeTag)) return;

	UInventoryHoverItem* HoverItem = GetHoverItem();
	
	// Create an Equipped Slotted Item and add it to the Equipped Grid Slot (call EquippedGridSlot->OnItemEquipped())
	const float TileSize = UInventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize();
	UInventoryEquipSlottedItem* EquippedSlottedItem = EquippedGridSlot->OnItemEquipped(
		HoverItem->GetInventoryItem(),
		EquipmentTypeTag,
		TileSize
	);
	EquippedSlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked);

	// Inform the server that we've equipped an item (potentially unequipping an item as well)
	UInventoryComp* InventoryComponent = UInventoryStatics::GetInventoryComp(GetOwningPlayer());
	check(IsValid(InventoryComponent)); 

	InventoryComponent->Server_EquipSlotClicked(HoverItem->GetInventoryItem(), nullptr);

	if (GetOwningPlayer()->GetNetMode() != NM_DedicatedServer)
	{
		InventoryComponent->OnItemEquipped.Broadcast(HoverItem->GetInventoryItem());
	}
	
	// Clear the Hover Item
	Grid_Equippables->ClearHoverItem();
}

void USpaitialInventoryComp::SetItemDescriptionSizeAndPosition(UItemDescriptionWidget* ItemDesc,
                                                               UCanvasPanel* Canvas)
{
	UCanvasPanelSlot* ItemDescriptionCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemDesc);
	if (!IsValid(ItemDescriptionCPS)) return;

	const FVector2D ItemDescriptionSize = ItemDesc->GetBoxSize();
	ItemDescriptionCPS->SetSize(ItemDescriptionSize);

	FVector2D ClampedPosition = UInventoryWidgetUtils::GetClampedWidgetPosition(
		UInventoryWidgetUtils::GetWidgetSize(Canvas),
		ItemDescriptionSize,
		UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer()));

	ItemDescriptionCPS->SetPosition(ClampedPosition);
	GEngine->AddOnScreenDebugMessage(-1,0.f,FColor::Red,FString::Printf(TEXT("Item Desc CPS zOrder:%d"),ItemDescriptionCPS->GetZOrder()));
	if (ItemDescriptionCPS->GetZOrder() == 0)
	{
		// temp zorder , will be use widget config to set
		ItemDescriptionCPS->SetZOrder(20);
	}
}

bool USpaitialInventoryComp::CanEquipHoverItem(UInventoryEquipmentSlot* EquippedGridSlot,
	const FGameplayTag& EquipmentTypeTag) const
{
	if (!IsValid(EquippedGridSlot) || IsValid(EquippedGridSlot->GetInventoryItem())) return false;

	UInventoryHoverItem* HoverItem = GetHoverItem();
	if (!IsValid(HoverItem)) return false;

	UInventoryItem* HeldItem = HoverItem->GetInventoryItem();

	return HasHoverItem() && IsValid(HeldItem) &&
		!HoverItem->GetIsStackable() &&
			HeldItem->GetItemManifest().GetItemCategory() == EInventoryGridType::Equippable &&
				HeldItem->GetItemManifest().GetGameplayTag().MatchesTag(EquipmentTypeTag);
}

UInventoryEquipmentSlot* USpaitialInventoryComp::FindSlotWithEquippedItem(UInventoryItem* EquippedItem) const
{
	auto* FoundEquippedGridSlot = EquippedGridSlots.FindByPredicate([EquippedItem](const UInventoryEquipmentSlot* GridSlot)
	{
		return GridSlot->GetInventoryItem() == EquippedItem;
	});
	return FoundEquippedGridSlot ? *FoundEquippedGridSlot : nullptr;
}

void USpaitialInventoryComp::ClearSlotOfItem(UInventoryEquipmentSlot* EquippedGridSlot)
{
	if (IsValid(EquippedGridSlot))
	{
		EquippedGridSlot->SetEquippedSlottedItem(nullptr);
		EquippedGridSlot->SetInventoryItem(nullptr);
	}
}

void USpaitialInventoryComp::RemoveEquippedSlottedItem(UInventoryEquipSlottedItem* EquippedSlottedItem)
{
	if (!IsValid(EquippedSlottedItem)) return;

	if (EquippedSlottedItem->OnEquippedSlottedItemClicked.IsAlreadyBound(this, &ThisClass::EquippedSlottedItemClicked))
	{
		EquippedSlottedItem->OnEquippedSlottedItemClicked.RemoveDynamic(this, &ThisClass::EquippedSlottedItemClicked);
	}
	EquippedSlottedItem->RemoveFromParent();
}

void USpaitialInventoryComp::MakeEquippedSlottedItem(UInventoryEquipSlottedItem* EquippedSlottedItem,
	UInventoryEquipmentSlot* EquippedGridSlot, UInventoryItem* ItemToEquip)
{
	if (!IsValid(EquippedGridSlot)) return;

	UInventoryEquipSlottedItem* SlottedItem = EquippedGridSlot->OnItemEquipped(
		ItemToEquip,
		EquippedSlottedItem->GetEquipmentTypeTag(),
		UInventoryStatics::GetInventoryWidget(GetOwningPlayer())->GetTileSize());
	if (IsValid(SlottedItem)) SlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &ThisClass::EquippedSlottedItemClicked);

	EquippedGridSlot->SetEquippedSlottedItem(SlottedItem);
}

void USpaitialInventoryComp::BroadcastSlotClickedDelegates(UInventoryItem* ItemToEquip,
	UInventoryItem* ItemToUnequip) const
{
	UInventoryComp* InventoryComponent = UInventoryStatics::GetInventoryComp(GetOwningPlayer());
	check(IsValid(InventoryComponent));
	InventoryComponent->Server_EquipSlotClicked(ItemToEquip, ItemToUnequip);
}

