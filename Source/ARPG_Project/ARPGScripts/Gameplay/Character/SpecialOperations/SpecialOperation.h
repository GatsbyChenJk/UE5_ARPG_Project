// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SpecialOperation.generated.h"

class AARPGBaseCharacter;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class ECharacterOperationState : uint8
{
	Inactive,    
	Active,      
	Cooldown,   
	Blocked     
};

UINTERFACE()
class USpecialOperation : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPG_PROJECT_API ISpecialOperation
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Character Operation")
	void InitOperation(AARPGBaseCharacter* OwnerCharacter, UAbilitySystemComponent* ASC);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Character Operation")
	bool StartedOperation();

	UFUNCTION(BlueprintNativeEvent, Category = "Character Operation")
	void OnGoingOperation();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Character Operation")
	void CompletedOperation(bool bIsInterrupted = false);

	UFUNCTION(BlueprintNativeEvent, Category = "Character Operation")
	void CanceledOperation();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Character Operation")
	void TriggeredOperation();
	
	UFUNCTION(BlueprintNativeEvent, Category = "Character Operation")
	ECharacterOperationState GetOperationState() const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Character Operation")
	FString GetOperationID() const;
};
