// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FWidgetManifest.h"
#include "BaseWidget.generated.h"

/**
 * 
 */



//封装所有 UI 的通用逻辑（全屏、打开 / 关闭、动画、输入处理）

UCLASS()
class ARPG_PROJECT_API UBaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FUIWidgetConfig WidgetConfig;

	UFUNCTION(blueprintCallable)
	void SetWidgetConfig(const FUIWidgetConfig& Config) { WidgetConfig = Config; }
	FUIWidgetConfig GetWidgetConfig() { return WidgetConfig; }

	// logic after construct
	virtual void OpenWidget();
	virtual void CloseWidget();

	// logic when toggled
	virtual void OnOpenWidget(APlayerController* OwningController);
	virtual void OnCloseWidget(APlayerController* OwningController);

	UFUNCTION(BlueprintCallable)
	virtual void ToggleWidget(APlayerController* OwningController);
protected:
	bool bIsWidgetOpen;
};

