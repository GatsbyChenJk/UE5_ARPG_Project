// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGLeaf.h"
#include "ARPGLeaf_CharaterText.generated.h"

class UTextBlock;
class UCharacterSelect;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLeaf_CharacterText : public UARPGLeaf
{
	GENERATED_BODY()
	virtual void NativePreConstruct() override;
public:
	UFUNCTION()
	void SetParentWidget(UCharacterSelect* Parent);

	void SetText_Name(const FText& Text, bool bCollapse);
	void SetText_Value(const FText& Text, bool bCollapse);
private:
	UFUNCTION()
	void OnCharacterChosen(const FString& CharacterID);
	
	UPROPERTY()
	TObjectPtr<UCharacterSelect> ParentWidget;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_AttributeName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_AttributeValue;

	UPROPERTY(EditAnywhere,Category="ARPG UI")
	FText AttributeName;

	UPROPERTY(EditAnywhere,Category="ARPG UI")
	FText AttributeNameToQuery;
};
