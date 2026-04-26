// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGLeaf.h"
#include "MiniMapWidget.generated.h"

class UImage;

UCLASS()
class ARPG_PROJECT_API UMiniMapWidget : public UARPGLeaf
{
    GENERATED_BODY()

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UImage> Image_Map;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UImage> Image_PlayerCursor;
};
