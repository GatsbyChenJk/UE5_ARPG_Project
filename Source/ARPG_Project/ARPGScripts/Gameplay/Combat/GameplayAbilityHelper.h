// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "AbilitySystemComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayAbilityHelper.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UGameplayAbilityHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DisplayName = "Grant Event Ability"))
	static FGameplayAbilitySpecHandle GrantEventAbility(
		UAbilitySystemComponent* ASC,
		TSubclassOf<UGameplayAbility> AbilityClass,
		FGameplayTag EventTag,
		int32 Level = 1
	);

	static void VerifyTriggerRegistration(
	UAbilitySystemComponent* ASC,
	FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayTag EventTag);
};
