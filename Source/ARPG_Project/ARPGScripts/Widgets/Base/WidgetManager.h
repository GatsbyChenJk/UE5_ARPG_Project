// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWidget.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WidgetManager.generated.h"

/**
 * 
 */

//负责 UI 的创建、销毁、切换，避免业务逻辑直接操作 UI，提升扩展性
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetRegistered);

UCLASS()
class ARPG_PROJECT_API UWidgetManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 打开指定UI（通过UIID）
	UFUNCTION(BlueprintCallable, Category = "UI_Mgr")
	UBaseWidget* OpenUI(const FString& WidgetID);

	UBaseWidget* OpenUI(const FString& WidgetID, APlayerController* InPC);

	// 关闭指定UI
	UFUNCTION(BlueprintCallable, Category = "UI_Mgr")
	void CloseUI(const FString& WidgetID);

	// 关闭所有UI
	UFUNCTION(BlueprintCallable, Category = "UI_Mgr")
	void CloseAllUI();

	// 获取已打开的UI
	UFUNCTION(BlueprintCallable, Category = "UI_Mgr")
	UBaseWidget* GetOpenedUIByID(const FString& WidgetID);

	void RegisterWidget(const FString& WidgetID,UBaseWidget* Widget);

	FOnWidgetRegistered OnWidgetRegistered;
private:
	// 已打开的UI缓存
	TMap<FString, UBaseWidget*> OpenedUIMap;
};
