// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMapPointGenerator.h"
#include "EscapePointGenerator.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UEscapePointGenerator : public UBaseMapPointGenerator
{
	GENERATED_BODY()
public:
	virtual TArray<FBasePointData> GetValidPoints_Implementation() override;
	
	UFUNCTION(BlueprintCallable, Category = "EscapePointGenerator")
	void InitEscapeParams(float InGameRunTime, bool bHasEscapeItem) 
	{
		GameRunTime = InGameRunTime;
		bPlayerHasEscapeItem = bHasEscapeItem;
	}

	UFUNCTION()
	void SetEscapePointActor(TSubclassOf<AActor> PortalActorClass)
	{
		PortalClass = PortalActorClass;
	}

	UFUNCTION()
	TSubclassOf<AActor> GetEscapePointActor()
	{
		return PortalClass;
	}

private:
	TSubclassOf<AActor> PortalClass;
	
	float GameRunTime = 0.f;
	
	bool bPlayerHasEscapeItem = false;
};
