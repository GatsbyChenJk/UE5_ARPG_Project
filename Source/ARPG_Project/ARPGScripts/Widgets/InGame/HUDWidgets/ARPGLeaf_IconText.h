// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGLeaf.h"
#include "ARPGLeaf_IconText.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLeaf_IconText : public UARPGLeaf
{
	GENERATED_BODY()

	virtual void NativePreConstruct() override;
public:
	void SetValueText(float InValue);
private:
	UPROPERTY(EditAnywhere,Category="ARPG UI")
	TObjectPtr<UTexture2D> Texture_Icon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Value;
};
