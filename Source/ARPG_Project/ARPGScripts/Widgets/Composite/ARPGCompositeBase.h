// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ARPGScripts/Widgets/Base/BaseWidget.h"
#include "Blueprint/UserWidget.h"
#include "ARPGCompositeBase.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGCompositeBase : public UBaseWidget
{
	GENERATED_BODY()
public:
	FGameplayTag GetQueryTag() const { return QueryTag; }
	void SetQueryTag(const FGameplayTag& Tag) { QueryTag = Tag; }
	virtual void Collapse();
	void Expand();

	using ARPGComPositeFunc = TFunction<void(UARPGCompositeBase*)>;
	virtual void ApplyFunction(ARPGComPositeFunc Function) {}
private:
	UPROPERTY(EditAnywhere, Category = "ARPG UI")
	FGameplayTag QueryTag;
};
