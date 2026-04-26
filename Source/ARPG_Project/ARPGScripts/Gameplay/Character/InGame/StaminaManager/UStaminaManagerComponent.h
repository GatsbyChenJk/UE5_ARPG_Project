#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "UStaminaManagerComponent.generated.h"

class AARPGBaseCharacter;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UStaminaManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaManagerComponent();

	bool IsValidStamina();

	UFUNCTION(Server, Reliable)
	void Server_InitStaminaManagerComponent();

	UFUNCTION(BlueprintCallable)
	void RemoveActiveRecoverStaminaGE();

	UFUNCTION(BlueprintCallable)
	void RemoveActiveConsumeStaminaGE();

	UFUNCTION()
	void ActivateRecoverStaminaGE();
protected:
	virtual void BeginPlay() override;

	// 监听的标签
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	FGameplayTag ConsumeTag;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	FGameplayTag RecoverTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	TSubclassOf<UGameplayEffect> StaminaRecoveryEffectClass;

	FActiveGameplayEffectHandle StaminaRecoveryEffectSpec;
private:
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	UPROPERTY()
	AARPGBaseCharacter* OwnerChar;
	
	void UpdateStaminaUI(const FOnAttributeChangeData& Data, float MaxStamina);
	
	void OnStaminaAttributeChanged(const FOnAttributeChangeData& Data);
	
};
