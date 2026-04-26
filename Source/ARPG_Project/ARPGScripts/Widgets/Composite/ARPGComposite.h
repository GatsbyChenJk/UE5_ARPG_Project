// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGCompositeBase.h"
#include "ARPGComposite.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGComposite : public UARPGCompositeBase
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void ApplyFunction(ARPGComPositeFunc Function) override;
	virtual void Collapse() override;
	TArray<UARPGCompositeBase*> GetChildren() { return Children; }

	UARPGCompositeBase* FindChildByTag(const FGameplayTag& QueryTag);
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UARPGCompositeBase>> Children;
};
