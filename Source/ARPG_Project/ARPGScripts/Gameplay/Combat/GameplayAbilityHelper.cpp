// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbilityHelper.h"

FGameplayAbilitySpecHandle UGameplayAbilityHelper::GrantEventAbility(UAbilitySystemComponent* ASC,
	TSubclassOf<UGameplayAbility> AbilityClass, FGameplayTag EventTag, int32 Level)
{
	if (!ASC || !AbilityClass) return FGameplayAbilitySpecHandle();
    
	// 创建AbilitySpec
	FGameplayAbilitySpec Spec(AbilityClass, Level, INDEX_NONE, ASC->GetOwner());
    
	// 添加事件触发器
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = EventTag;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	
    
	// 授予能力
	FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
    
	// 调试信息
	if (Handle.IsValid())
	{
		//VerifyTriggerRegistration(ASC, Handle, EventTag);
		
		UE_LOG(LogTemp, Log, TEXT("Granted event ability with trigger: %s"), 
			*EventTag.ToString());
	}
    
	return Handle;
}

void UGameplayAbilityHelper::VerifyTriggerRegistration(
	UAbilitySystemComponent* ASC,
	FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayTag EventTag)
{
	if (!ASC) return;
    
	// 使用反射访问私有成员GameplayEventTriggeredAbilities
	static FName GameplayEventTriggeredAbilitiesName = TEXT("GameplayEventTriggeredAbilities");
	FProperty* Property = ASC->GetClass()->FindPropertyByName(GameplayEventTriggeredAbilitiesName);
    
	if (Property)
	{
		// 获取映射表
		TMap<FGameplayTag, TArray<FGameplayAbilitySpecHandle>>* TriggeredAbilities = 
			Property->ContainerPtrToValuePtr<TMap<FGameplayTag, TArray<FGameplayAbilitySpecHandle>>>(ASC);
        
		if (TriggeredAbilities)
		{
			// 检查我们的EventTag是否在映射表中
			if (TriggeredAbilities->Contains(EventTag))
			{
				UE_LOG(LogTemp, Log, TEXT("Trigger %s registered in GameplayEventTriggeredAbilities"), 
					*EventTag.ToString());
                
				// 检查我们的能力句柄是否在列表中
				TArray<FGameplayAbilitySpecHandle>& Handles = (*TriggeredAbilities)[EventTag];
				bool bFound = false;
				for (const FGameplayAbilitySpecHandle& Handle : Handles)
				{
					if (Handle == AbilityHandle)
					{
						bFound = true;
						break;
					}
				}
                
				if (bFound)
				{
					UE_LOG(LogTemp, Log, TEXT("Ability handle found in trigger list"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Ability handle NOT found in trigger list"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Trigger %s NOT found in GameplayEventTriggeredAbilities"), 
					*EventTag.ToString());
			}
		}
	}
}
