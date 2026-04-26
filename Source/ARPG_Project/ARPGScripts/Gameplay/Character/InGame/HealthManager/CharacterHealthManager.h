// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"
#include "Components/ActorComponent.h"
#include "CharacterHealthManager.generated.h"


class AARPGBaseCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UCharacterHealthManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterHealthManager();

	bool IsValidHealth();

	UFUNCTION(Server, Reliable)
	void Server_InitHealthManagerComponent();


protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	UPROPERTY()
	AARPGBaseCharacter* OwnerChar;
	
	void UpdateHealthUI(const FOnAttributeChangeData& Data, float MaxHealth);
	
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
};
