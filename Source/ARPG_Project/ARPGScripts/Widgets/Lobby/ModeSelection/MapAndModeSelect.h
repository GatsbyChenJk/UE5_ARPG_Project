// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGComposite.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "MapAndModeSelect.generated.h"

class UButton;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UMapAndModeSelect : public UARPGComposite
{
	GENERATED_BODY()
protected:
	virtual void NativeOnInitialized() override;
public:
	void SetCurrentMapID(FString InMapID) { CurrentMapID = InMapID; };
	void SetPreviewMapName(FText InText) {Text_PreviewMap->SetText(InText);}
	void SetPreviewModeName(FText InText) {Text_PreviewMode->SetText(InText);}
	void SetPreviewMapIcon(UTexture2D* InTexture)
	{
		Image_PreviewMapIcon->SetBrushFromTexture(InTexture);
	};
private:
	UFUNCTION()
	void OnConfirmCurrentChoice();
	
	FString CurrentMapID;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_PreviewMap;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_PreviewMode;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_PreviewMapIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Confirm;
};
