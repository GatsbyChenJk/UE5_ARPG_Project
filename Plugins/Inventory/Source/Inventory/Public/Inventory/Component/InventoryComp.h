// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/FastArray/InventoryFastArray.h"
#include "Types/InventoryGridType.h"
#include "InventoryComp.generated.h"


class UARPGEventData;
class AAIController;
class UItemComponent;
class UInventoryItem;
class UInventoryBaseComp;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChanged,UInventoryItem*,Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStackChanged,const FInventorySlotAvailabilityResult&,Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemEquipStatusChanged, UInventoryItem*, Item);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMenuToggled, bool, bOpen);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),Blueprintable)
class INVENTORY_API UInventoryComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComp();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly,Category = "Inventory")
	void TryAddItem(UItemComponent* ItemComp);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerAIAddNewItem(UItemComponent* ItemComponent);

	UFUNCTION(Server,Reliable)
	void ServerAddNewItem(UItemComponent* ItemComp,int32 StackCount,int32 Remainder);

	UFUNCTION(Server,Reliable)
	void ServerAddStackToItem(UItemComponent* ItemComp,int32 StackCount,int32 Remainder);

	UFUNCTION(Server,Reliable)
	void Server_DropItem(UInventoryItem* Item,int32 StackCount);

	void SpawnDroppedItem(UInventoryItem* Item,int32 StackCount);

	UFUNCTION(Server,Reliable)
	void Server_ConsumeItem(UInventoryItem* Item);

	UFUNCTION(Server,Reliable)
	void Server_SellItem(UInventoryItem* Item);

	UFUNCTION(Server,Reliable)
	void Server_SellAllItem();

	UFUNCTION(Server, Reliable)
	void Server_EquipSlotClicked(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipSlotClicked(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip);

	UFUNCTION(BlueprintCallable,Server, Reliable)
	void Server_AIEquipWeapon(FGameplayTag ItemType);
	
	UFUNCTION(BlueprintCallable,NetMulticast, Reliable)
	void Multicast_AIEquipWeapon(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip);

	void AddRepSubObj(UObject* SubObj);

	UInventoryBaseComp* GetInventoryMenu() const { return InventoryMenu; };

	UInventoryItem* FindItemByType(FGameplayTag ItemType);

	FInventoryItemChanged OnItemAdded;
	FInventoryItemChanged OnItemRemoved;
	FNoRoomInInventory OnNoRoomInInventory;
	FOnStackChanged OnStackChange;
	FItemEquipStatusChanged OnItemEquipped;
	FItemEquipStatusChanged OnItemUnequipped;
	//FInventoryMenuToggled OnInventoryMenuToggled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float DropSpawnAngleMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float DropSpawnAngleMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float DropSpawnDistanceMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float DropSpawnDistanceMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	float RelativeSpawnElevation;

protected:
	virtual void BeginPlay() override;
private:
	UFUNCTION()
	void ARPG_AIAddEquipWeapon(UARPGEventData* Event);

	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningController;

	UPROPERTY()
	TWeakObjectPtr<AAIController> OwningAIController;
	
	UPROPERTY()
	TObjectPtr<UInventoryBaseComp> InventoryMenu;

	UPROPERTY(Replicated)
	FInventoryFastArray InventoryList;
	
};
