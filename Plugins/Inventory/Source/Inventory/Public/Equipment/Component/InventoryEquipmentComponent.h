// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "InventoryEquipmentComponent.generated.h"

class AAIController;
class AIController;
struct FItemManifest;
struct FEquipmentFragment;
class AInventoryEquipActor;
class UInventoryItem;
class UInventoryComp;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORY_API UInventoryEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryEquipmentComponent();

	void SetOwningSkeletalMesh(USkeletalMeshComponent* OwningMesh);
	void SetIsProxy(bool bProxy) { bIsProxy = bProxy; }
	void InitializeOwner(APlayerController* PlayerController);
	
protected:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	TWeakObjectPtr<UInventoryComp> InventoryComponent;
	// if Owner is Player, this is valid
	TWeakObjectPtr<APlayerController> OwningPlayerController;
	// if Owner is AI,this is valid
	TWeakObjectPtr<AAIController> OwningAIController;

	TWeakObjectPtr<USkeletalMeshComponent> OwningSkeletalMesh;

	UFUNCTION()
	void OnItemEquipped(UInventoryItem* EquippedItem);

	UFUNCTION(Server, Reliable)
	void Server_OnItemEquipped(UInventoryItem* EquippedItem);

	UFUNCTION()
	void OnItemUnequipped(UInventoryItem* UnequippedItem);

	UFUNCTION()
	void OnAIItemEquipped(UInventoryItem* EquippedItem);

	UFUNCTION()
	void OnAIItemUnequipped(UInventoryItem* UnequippedItem);

	void InitPlayerController();
	void InitAIController();
	void InitInventoryComponent();
	void InitAIInventoryComponent();
	AInventoryEquipActor* SpawnEquippedActor(FEquipmentFragment* EquipmentFragment, const FItemManifest& Manifest, USkeletalMeshComponent* AttachMesh);
	AInventoryEquipActor* SpawnEquippedActorForProxy(FEquipmentFragment* EquipmentFragment, const FItemManifest& Manifest, USkeletalMeshComponent* AttachMesh);

	UPROPERTY()
	TArray<TObjectPtr<AInventoryEquipActor>> EquippedActors;

	AInventoryEquipActor* FindEquippedActor(const FGameplayTag& EquipmentTypeTag);
	
	void RemoveEquippedActor(const FGameplayTag& EquipmentTypeTag);

	UFUNCTION()
	void OnPossessedPawnChange(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void OnAIPossessedPawnChange(APawn* OldPawn, APawn* NewPawn);

	UPROPERTY(Replicated)
	bool bIsProxy{false};
};
