// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputTriggers.h"
#include "Engine/DataAsset.h"
#include "BaseInputConfig.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FInputBindingEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Binding")
	TSubclassOf<UObject> TargetClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "Binding")
	UInputAction* InputAction = nullptr;

	UPROPERTY(EditAnywhere, Category = "Binding")
	ETriggerEvent TriggerEvent = ETriggerEvent::Triggered; 

	UPROPERTY(EditAnywhere, Category = "Binding")
	FName FunctionName;
};


UCLASS()
class ARPG_PROJECT_API UBaseInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Actions")
	TSet<TObjectPtr<UInputAction>> AllInputActions;
	
	UPROPERTY(EditAnywhere, Category = "Bindings")
	TArray<FInputBindingEntry> Bindings;
};
