// Fill out your copyright notice in the Description page of Project Settings.


#include "InputBindingHelper.h"

#include "EnhancedInputComponent.h"

void UInputBindingHelper::BindInput(UEnhancedInputComponent* InputComponent, UObject* BindObject,
                                    const UBaseInputConfig* Config)
{
	if (!InputComponent || !BindObject || !Config) return;

	// 1. 绑定对象自身
	BindInputInternal(InputComponent, BindObject, Config);

	// 2. 若对象是Actor，则递归绑定所有组件
	if (AActor* Actor = Cast<AActor>(BindObject))
	{
		for (UActorComponent* Comp : Actor->GetComponents())
		{
			if (Comp)
			{
				BindInputInternal(InputComponent, Comp, Config);
			}
		}
	}
}

void UInputBindingHelper::BindInputInternal(UEnhancedInputComponent* InputComponent, UObject* Target,
	const UBaseInputConfig* Config)
{
	UClass* TargetClass = Target->GetClass();

	for (const FInputBindingEntry& Entry : Config->Bindings)
	{
		// 检查目标对象的类是否匹配条目指定的类（支持继承）
		if (Entry.TargetClass && TargetClass->IsChildOf(Entry.TargetClass))
		{
			if (Entry.InputAction && !Entry.FunctionName.IsNone())
			{
				// 可选：验证函数是否存在
				if (UFunction* Func = Target->FindFunction(Entry.FunctionName))
				{
					InputComponent->BindAction(Entry.InputAction, Entry.TriggerEvent, Target, Entry.FunctionName);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("InputBinder: Function '%s' not found on %s"), 
						*Entry.FunctionName.ToString(), *Target->GetName());
				}
			}
		}
	}
}
