// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ARPGScripts/Gameplay/Base/SaveGameSystem/ARPGSaveGameFunc.h"

#include "InventoryPlayerController.generated.h"

/**
 * 
 */

class UARPGEventData;
class UInGameWidgetCompoent;
class UCharacterChooseWidget;
class UInventoryComp;
class UInputMappingContext;
class UInputAction;

UCLASS()
class INVENTORY_API AInventoryPlayerController : public APlayerController,public IARPGSaveGameFunc
{
	GENERATED_BODY()

	AInventoryPlayerController();
protected:
	virtual void BeginPlay() override;

	virtual  void Tick(float DeltaSeconds) override;

	virtual void SetupInputComponent() override;
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ARPG UI")
	FGameplayTag HealthQueryTag;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ARPG UI")
	FGameplayTag StaminaQueryTag;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ARPG UI")
	FGameplayTag MoneyQueryTag;
	
	UFUNCTION(BlueprintCallable)
	void ToggleInventory();

	UInGameWidgetCompoent* GetInGameWidgetComponent();

	UFUNCTION(BlueprintCallable)
	void ShowInteractMessage();

	//UFUNCTION(BlueprintCallable)
	void ShowInteractMessage(const FString& Message);

	UFUNCTION(BlueprintCallable)
	void HideInteractMessage();

	UFUNCTION()
	void OnCharacterDataSaved(bool bSuccess,int32 UserIndex);

	UFUNCTION(Server,Reliable)
	void Server_TravelToLobby();
private:
	// HUD Update start
	UFUNCTION()
	void ARPG_OnHealthChanged(UARPGEventData* EventData);

	UFUNCTION()
	void ARPG_OnStaminaChanged(UARPGEventData* EventData);

	UFUNCTION()
	void ARPG_OnMoneyChanged(UARPGEventData* EventData);
	// HUD Update end
	
	UFUNCTION()
	void ARPG_CharacterRespawn(UARPGEventData* EventData);
	
	void PrimaryInteract();

	void TraceForItem();

	void SaveGameInteract();

	virtual void SaveCharacterData_Implementation(AARPGBaseCharacter* InPlayer) override;

	TWeakObjectPtr<UInventoryComp> InventoryComp;

	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditDefaultsOnly,Category = "Inventory | Interact")
	TObjectPtr<UInputAction> PrimaryInteraction;

	UPROPERTY(EditDefaultsOnly,Category = "Inventory | Interact")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	UPROPERTY(EditDefaultsOnly,Category = "Inventory | Interact")
	TObjectPtr<UInputAction> SaveGameAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory | Interact")
	float TraceDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory | Trace")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurActor;

	UPROPERTY()
	TWeakObjectPtr<AActor> PrevActor;

	UPROPERTY()
	UInGameWidgetCompoent* InGameWidgetComponent;
};
