// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGLeaf.h"
#include "ARPGLeaf_ModeChoice.generated.h"

class UTextBlock;
class UButton;
class UMapAndModeSelect;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLeaf_ModeChoice : public UARPGLeaf
{
	GENERATED_BODY()
	virtual void NativeOnInitialized() override;
	virtual void NativePreConstruct() override;
public:
	void SetParentWidget(UMapAndModeSelect* Parent) {ParentWidget = Parent;};

	UFUNCTION()
	void OnWidgetClicked();
private:
	UPROPERTY(EditAnywhere,Category="ARPG UI |Mode Choice")
	FText ModeName = FText::GetEmpty();

	UPROPERTY()
	TObjectPtr<UMapAndModeSelect> ParentWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Mode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_ModeChoose;
};
