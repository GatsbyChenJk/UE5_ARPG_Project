// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ARPGScripts/Gameplay/AIController/InGameAIController.h"
#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Base/AbilitySystem/ARPGAbilitySystemComponent.h"
#include "InGameAICharacter.generated.h"

class UARPGEventData;
class UARPGEventData_Weapon;
class UAIWidgetComponent;
class UWidgetComponent;
struct FAIManifest;

UCLASS()
class ARPG_PROJECT_API AInGameAICharacter : public AARPGBaseCharacter
{
	GENERATED_BODY()
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	AInGameAICharacter();

	/** 获取AI阵营ID（重写基类，返回敌人阵营1） */
	virtual int32 GetTeamID() const override { return 1; }

public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	// GAS start
	/** 返回原生 ASC 基类指针（兼容 IAbilitySystemInterface / 现有调用方） */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return Cast<UAbilitySystemComponent>(AIAbilitySystemComp); }

	/** 返回类型安全的 ARPG ASC，供装备/蒙太奇系统使用 */
	virtual UARPGAbilitySystemComponent* GetARPGAbilitySystemComponent() const override { return AIAbilitySystemComp; }

	virtual const UInGameCharacterAttributeSet* GetAttributeSet() const override { return AIAttributeSet; }

	void AIBasicDataInitialize(FAIManifest AIConfigData);

	void AICharacterDataInit(FAIManifest AIConfigData);

	void AICharacterAbilitiesInit(FAIManifest AIConfigData);

	UFUNCTION()
	void ARPG_AIEquipWeapon(UARPGEventData_Weapon* Event);

	UFUNCTION()
	void ARPG_AIUnequipWeapon(UARPGEventData_Weapon* Event);

	UFUNCTION()
	void ARPG_OnHealthChanged(UARPGEventData* Event);

	UFUNCTION()
	void ARPG_OnStaminaChanged(UARPGEventData* Event);
	// GAS end

	void AIEquipmentInitialize(FAIManifest AIConfigData);

	// AI logic start
	virtual void CharacterDeath() override;
	
	UFUNCTION(BlueprintCallable, Category = "AI|Controller")
	AInGameAIController* GetAIController() const { return Cast<AInGameAIController>(GetController()); }

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	// AI logic end
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float DefaultAttackRange = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float AttackCooldown = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	TArray<FGameplayAbilitySpecHandle> AIGrantedAbilities;

	UFUNCTION(BlueprintCallable, Category = "AI| Getter")
	UAIWidgetComponent* GetWidgetComponent() { return AIWidgetComponent;};

	UFUNCTION()
	TArray<AActor*> GetEquipmentActors() {return EquipmentActors;};
private:
	// UI start
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAIWidgetComponent* AIWidgetComponent;
	// UI end
	
	// GAS start
	UARPGAbilitySystemComponent* AIAbilitySystemComp;

	const UInGameCharacterAttributeSet* AIAttributeSet;
	// GAS end

	// Equipment start
	UPROPERTY(Replicated)
	TArray<AActor*> EquipmentActors;
	// Equipment end
};
