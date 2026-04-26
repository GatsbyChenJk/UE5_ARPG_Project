// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGComposite.h"
#include "ARPGCharacterComposite.generated.h"

class UARPGLeaf;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGCharacterComposite : public UARPGComposite
{
	GENERATED_BODY()

public:
	virtual void OnOpenWidget(APlayerController* OwningController) override;
	virtual void OnCloseWidget(APlayerController* OwningController) override;
private:
};
