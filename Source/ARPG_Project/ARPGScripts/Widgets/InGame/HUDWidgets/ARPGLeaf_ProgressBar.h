// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGLeaf.h"
#include "ARPGLeaf_ProgressBar.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLeaf_ProgressBar : public UARPGLeaf
{
	GENERATED_BODY()

public:
	void SetPercent(float Percent);
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_LeafBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_BufferedBar;
	
	float TargetPercent = 0.0f;
	
	float CurrentBufferedPercent = 1.0f;
	
	FTimerHandle BufferedTimerHandle;

	UPROPERTY(EditAnywhere, Category = "ProgressBar")
	float BufferedSpeed = 2.0f;

	UFUNCTION()
	void UpdateBufferedPercent();
};
