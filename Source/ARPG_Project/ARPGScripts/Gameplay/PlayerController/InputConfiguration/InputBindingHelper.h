// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInputConfig.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputBindingHelper.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UInputBindingHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// 主入口：为指定对象绑定输入（自动处理组件）
	UFUNCTION(BlueprintCallable, Category = "Input")
	static void BindInput(UEnhancedInputComponent* InputComponent, UObject* BindObject, const UBaseInputConfig* Config);

	static void BindInputInternal(UEnhancedInputComponent* InputComponent, UObject* Target, const UBaseInputConfig* Config);
};
