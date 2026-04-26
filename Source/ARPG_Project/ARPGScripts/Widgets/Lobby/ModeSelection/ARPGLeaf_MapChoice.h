// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGLeaf.h"
#include "ARPGLeaf_MapChoice.generated.h"

class UButton;
class UImage;
class UMapAndModeSelect;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLeaf_MapChoice : public UARPGLeaf
{
	GENERATED_BODY()
	virtual void NativePreConstruct() override;
public:
	virtual void NativeOnInitialized() override;
	UFUNCTION()
	void SetParentWidget(UMapAndModeSelect* Parent);

	UFUNCTION()
	void OnWidgetClicked();
private:
	UPROPERTY(EditAnywhere,Category="ARPG UI | Map Config")
	FString MapID;

	UPROPERTY(EditAnywhere,Category="ARPG UI | Map Config")
	FString MapName;
	
	UPROPERTY(EditAnywhere,Category="ARPG UI | Map Config")
	TObjectPtr<UTexture2D> Texture_MapIcon;

	UPROPERTY()
	TObjectPtr<UMapAndModeSelect> ParentWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_MapIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_MapChoose;	
};
