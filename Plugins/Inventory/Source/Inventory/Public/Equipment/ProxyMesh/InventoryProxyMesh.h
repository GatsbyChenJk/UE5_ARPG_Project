// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Component/InventoryEquipmentComponent.h"
#include "GameFramework/Actor.h"
#include "InventoryProxyMesh.generated.h"

UCLASS()
class INVENTORY_API AInventoryProxyMesh : public AActor
{
	GENERATED_BODY()

public:
	AInventoryProxyMesh();

	USkeletalMeshComponent* GetMesh() const { return Mesh; }

protected:
	virtual void BeginPlay() override;
	
	// This is the mesh on the player-controlled Character.
	TWeakObjectPtr<USkeletalMeshComponent> SourceMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInventoryEquipmentComponent> EquipmentComponent;

	// This is the proxy mesh we will see in the Inventory Menu.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	FTimerHandle TimerForNextTick;
	void DelayedInitializeOwner();
	void DelayedInitialization();
};
