// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "ARPGScripts/Gameplay/Base/AbilitySystem/ARPGAbilitySystemComponent.h"
#include "ARPGScripts/Gameplay/Character/FCharacterManifest.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "AInGamePlayerState.generated.h"

class UInGamePlayerConfig;
struct FCharacterData;
class UInGameCharacterAttributeSet;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API AAInGamePlayerState : public APlayerState,public IAbilitySystemInterface
{
	GENERATED_BODY()

	AAInGamePlayerState();
public:
	void InitializeAttributesForClient(ACharacter* InGameCharacter);
	

	void InitializeASCForPlayer(AController* OwningController,APawn* AvatarPawn);

	void ResetPlayer();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 返回类型安全的 ARPG 自定义 ASC，供需要 ARPG 扩展功能（Socket 复制、多 Mesh 蒙太奇）的系统使用 */
	UFUNCTION(BlueprintCallable, Category = "GAS")
	UARPGAbilitySystemComponent* GetARPGAbilitySystemComponent() const;

	const UInGameCharacterAttributeSet* GetAttributeSet() const {return AttributeSet;};

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// TODO : add some config var for sprint, attack , etc..
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="ID Config")
	UInGamePlayerConfig* IDConfigDataAsset;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UARPGAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	const UInGameCharacterAttributeSet* AttributeSet;
	
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_SetASCInitializedOnServer)
	bool bInitialized = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

private:
	TArray<FGameplayAbilitySpecHandle> GrantedAbilities;

	void CharacterAbilityInitOnServer(FCharacterData InData);

	// default attributes initialize
	UFUNCTION()
	void OnRep_SetASCInitializedOnServer();
	
	void ApplyGE_NoModifier(TSubclassOf<UGameplayEffect> InEffect);
	void ApplyGE_WithModifier(TSubclassOf<UGameplayEffect> InEffect, TArray<FGameplayTag> InTags,TArray<float> InMagnitude);

	void CharacterAttributesInitOnServer(FCharacterData InData);
	void CharacterAttributeBuffInitOnServer(FCharacterData InData);
	void ApplyInitialAttributesFromData(FCharacterData CharacterData);
	void ApplySpecialAbilitiesFromComponents(FCharacterData CharData, AAInGameCharacter* PlayerChar);
};
