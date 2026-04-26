// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGLeaf.h"
#include "ARPGLeaf_CharacterChoice.generated.h"

class UButton;
class UImage;
class UCharacterSelect;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLeaf_CharacterChoice : public UARPGLeaf
{
	GENERATED_BODY()
	virtual void NativePreConstruct() override;
public:
	virtual void NativeOnInitialized() override;
	UFUNCTION()
	void SetParentWidget(UCharacterSelect* Parent);

	UFUNCTION()
	void OnWidgetClicked();

private:
	UPROPERTY(EditAnywhere,Category="ARPG UI | Character Config")
	FString CharacterID;
	
	UPROPERTY(EditAnywhere,Category="ARPG UI | Character Config")
	TObjectPtr<UTexture2D> Texture_CharacterIcon;

	UPROPERTY()
	TObjectPtr<UCharacterSelect> ParentWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_CharacterIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_CharacterChoose;	
};
