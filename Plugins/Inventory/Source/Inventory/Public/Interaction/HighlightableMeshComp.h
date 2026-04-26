// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Highlightable.h"
#include "Components/StaticMeshComponent.h"
#include "HighlightableMeshComp.generated.h"

/**
 * 
 */
UCLASS()
class INVENTORY_API UHighlightableMeshComp : public UStaticMeshComponent,public IHighlightable
{
	GENERATED_BODY()
public:
	virtual void Highlight_Implementation() override;
	virtual void UnHighlight_Implementation() override;

	UPROPERTY(EditAnywhere,Category="Inventory")
	TObjectPtr<UMaterialInterface> HighlightMaterial;
};
