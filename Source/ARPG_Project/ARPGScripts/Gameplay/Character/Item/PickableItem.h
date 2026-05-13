// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ARPGScripts/Gameplay/Base/ARPGObjectPoolSystem/PoolableActor.h"
#include "PickableItem.generated.h"

UCLASS()
class ARPG_PROJECT_API APickableItem : public AActor, public IPoolableActor
{
	GENERATED_BODY()

public:
	APickableItem();

	virtual void OnActivateFromPool_Implementation(const FTransform& Transform) override;
	virtual void OnReturnToPool_Implementation() override;
	virtual bool IsActiveInPool_Implementation() const override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	bool bIsPooledActive = false;
};
