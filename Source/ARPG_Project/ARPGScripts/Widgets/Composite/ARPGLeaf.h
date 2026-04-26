// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGCompositeBase.h"
#include "ARPGLeaf.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLeaf : public UARPGCompositeBase
{
	GENERATED_BODY()
public:
	virtual void ApplyFunction(ARPGComPositeFunc Function) override;
};
