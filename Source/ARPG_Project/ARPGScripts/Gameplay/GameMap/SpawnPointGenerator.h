// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMapPointGenerator.h"
#include "SpawnPointGenerator.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API USpawnPointGenerator : public UBaseMapPointGenerator
{
	GENERATED_BODY()
public:
	virtual TArray<FBasePointData> GetValidPoints_Implementation() override;
	
	UFUNCTION(BlueprintCallable, Category = "SpawnPointGenerator")
	void SetPlayerCamp(bool bIsSelfCamp) { bCurrentSelfCamp = bIsSelfCamp; }

private:
	bool bCurrentSelfCamp = true;
};
