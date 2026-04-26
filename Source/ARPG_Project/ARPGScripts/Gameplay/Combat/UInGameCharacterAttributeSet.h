#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/FARPGEventData.h"
#include "UInGameCharacterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaAttributeChanged, float, NewStamina, float, OldStamina);

UCLASS()
class ARPG_PROJECT_API UInGameCharacterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	// 生命值属性
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, Health) // 生成Get/Set函数

	// 最大生命值属性
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_DefensePower)
	FGameplayAttributeData DefensePower;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, DefensePower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_DamageReduction)
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, DamageReduction)
	
	// 攻击力属性
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AttackPower)
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, AttackPower)

	// 体力属性
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AttackStaminaCost)
	FGameplayAttributeData AttackStaminaCost;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, AttackStaminaCost)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_DefenseStaminaCost)
	FGameplayAttributeData DefenseStaminaCost;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, DefenseStaminaCost)

	// 统计属性
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Money)
	FGameplayAttributeData CharacterMoney;
	ATTRIBUTE_ACCESSORS(UInGameCharacterAttributeSet, CharacterMoney)
protected:
	// 网络复制回调
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
	UFUNCTION()
	virtual void OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower);
	UFUNCTION()
	virtual void OnRep_DamageReduction(const FGameplayAttributeData& OldDamageReduction);

	UFUNCTION()
	virtual void OnRep_AttackStaminaCost(const FGameplayAttributeData& OldStaminaCost);

	UFUNCTION()
	virtual void OnRep_DefenseStaminaCost(const FGameplayAttributeData& OldStaminaCost);

	UFUNCTION()
	virtual void OnRep_Money(const FGameplayAttributeData& OldMoney);
	// 实现网络复制
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//void SendAttributeChangeEvent_Stamina(FName EventName,UARPGEventData_OnCharacterAttributeChanged* EventData);
	void SendAttributeChangeEvent(FName EventName,UARPGEventData_OnCharacterAttributeChanged* EventData);
};