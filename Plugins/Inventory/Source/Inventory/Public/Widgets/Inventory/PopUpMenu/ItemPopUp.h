// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemPopUp.generated.h"

class USizeBox;
class USlider;
class UTextBlock;
class UButton;
/**
 * 
 */

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPopupMenuSplit,int32,SplitAmount,int32,Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopupMenuDrop,int32,Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopupMenuConsume,int32,Index);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPopupMenuSell,int32,Index);

UCLASS()
class INVENTORY_API UItemPopUp : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void CollapseSellButton();
	void CollapseSplitButton() const;
	void CollapseConsumeButton() const;
	void SetSliderParams(const float Max,const float Value);
	FVector2D GetBoxSize() const;
	void SetGridIndex(const int32 Index) {GridIndex = Index;};
	int32 GetGridIndex() const { return GridIndex; };

	FPopupMenuSplit OnSplit;
	FPopupMenuDrop OnDrop;
	FPopupMenuConsume OnConsume;
	FPopupMenuSell OnSell;
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Sell;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Drop;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Consume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_Split;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	int32 GridIndex{INDEX_NONE};

	int32 GetSplitAmount() const;

	UFUNCTION()
	void OnSellButtonClicked();

	UFUNCTION()
	void OnDropButtonClicked();

	UFUNCTION()
	void OnConsumeButtonClicked();

	UFUNCTION()
	void OnSplitButtonClicked();
	
	UFUNCTION()
	void OnSliderValueChanged(float Value);
	
};
