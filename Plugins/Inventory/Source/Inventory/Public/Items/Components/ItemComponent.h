// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Manifest/ItemManifest.h"
#include "ItemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),Blueprintable)
class INVENTORY_API UItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	FString GetPickUpMessage() const {return this->PickUpMessage;};

	UFUNCTION()
	FItemManifest GetItemManifest() const {return this->ItemManifest;};

	UFUNCTION()
	void PickUp();
	
	void InitItemManifest(FItemManifest CopyOfManifest);

	UFUNCTION(BlueprintNativeEvent,Category="Inventory")
	void OnPickUp();
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickUpMessage;

	UPROPERTY(Replicated,EditAnywhere, Category = "Inventory")
	FItemManifest ItemManifest;
};
