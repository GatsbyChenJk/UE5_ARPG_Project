// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerWidgetComponent.generated.h"


class UBaseWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UPlayerWidgetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerWidgetComponent();

	UFUNCTION(BlueprintCallable)
	FString GetInteractMessage() const {return this->InteractMessage;};

	void SetSceneInteractable(bool bInteract) { bIsSceneObjectInteractable = bInteract;};

	bool GetSceneInteractable() { return bIsSceneObjectInteractable;};

	void SetItemInteractable(bool bInteract) { bIsItemInteractable = bInteract;};
	bool GetItemInteractable() { return bIsItemInteractable;};

	UBaseWidget* GetLoadingWidget();

	UFUNCTION()
	virtual void ToggleWidget(const FString& WidgetID);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void ConstructAllWidgets();

	virtual UBaseWidget* ConstructWidget(const FString& WidgetID);

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString InteractMessage;

	// Pre-created loading widget
	UPROPERTY()
	TObjectPtr<UBaseWidget> LoadingWidget;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningController;

private:
	bool bIsSceneObjectInteractable = false;
	bool bIsItemInteractable = false;
};
