// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "AIWidgetComponent.generated.h"


class UARPGLeaf_ProgressBar;
class UARPGEventData;
class UTargetLockWidget;
class UCharacterProgressBar;
class AInGameAICharacter;
class UProgressBar;
class UWidgetComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UAIWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UAIWidgetComponent();

	void SetHealthPercent(float HealthPercent);
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UARPGLeaf_ProgressBar> HealthBarWidget;

	UPROPERTY()
	TObjectPtr<AInGameAICharacter> OwnerAIChar;
	//TODO:
	// open widget when play lock on
	// close widget when player leave range 
};
