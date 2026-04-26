// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/PopUpMenu/ItemPopUp.h"

#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"

void UItemPopUp::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Consume->OnClicked.AddDynamic(this,&ThisClass::OnConsumeButtonClicked);
	Button_Drop->OnClicked.AddDynamic(this,&ThisClass::OnDropButtonClicked);
	Button_Split->OnClicked.AddDynamic(this,&ThisClass::OnSplitButtonClicked);
	Button_Sell->OnClicked.AddDynamic(this,&ThisClass::OnSellButtonClicked);
	Slider_Split->OnValueChanged.AddDynamic(this,&ThisClass::OnSliderValueChanged);

}

void UItemPopUp::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	RemoveFromParent();
}

void UItemPopUp::CollapseSellButton()
{
	Button_Sell->SetVisibility(ESlateVisibility::Collapsed);
}

void UItemPopUp::CollapseSplitButton() const
{
	Button_Split->SetVisibility(ESlateVisibility::Collapsed);
	Slider_Split->SetVisibility(ESlateVisibility::Collapsed);
	Text_Split->SetVisibility(ESlateVisibility::Collapsed);
}

void UItemPopUp::CollapseConsumeButton() const
{
	Button_Consume->SetVisibility(ESlateVisibility::Collapsed);
}

void UItemPopUp::SetSliderParams(const float Max, const float Value)
{
	Slider_Split->SetMaxValue(Max);
	Slider_Split->SetMinValue(1);
	Slider_Split->SetValue(Value);
	Text_Split->SetText(FText::AsNumber(FMath::Floor(Value)));
}

FVector2D UItemPopUp::GetBoxSize() const
{
	return FVector2D(SizeBox_Root->GetWidthOverride(),SizeBox_Root->GetHeightOverride());
}

int32 UItemPopUp::GetSplitAmount() const
{
	return FMath::Floor(Slider_Split->GetValue());
}

void UItemPopUp::OnSellButtonClicked()
{
	if (OnSell.ExecuteIfBound(GridIndex))
	{
		RemoveFromParent();
	}
}

void UItemPopUp::OnDropButtonClicked()
{
	if (OnDrop.ExecuteIfBound(GridIndex))
	{
		RemoveFromParent();
	}
}

void UItemPopUp::OnConsumeButtonClicked()
{
	if (OnConsume.ExecuteIfBound(GridIndex))
	{
		RemoveFromParent();
	}
}

void UItemPopUp::OnSplitButtonClicked()
{
	if (OnSplit.ExecuteIfBound(GetSplitAmount(),GridIndex))
	{
		RemoveFromParent();
	}
}

void UItemPopUp::OnSliderValueChanged(float Value)
{
	Text_Split->SetText(FText::AsNumber(FMath::Floor(Value)));
}
