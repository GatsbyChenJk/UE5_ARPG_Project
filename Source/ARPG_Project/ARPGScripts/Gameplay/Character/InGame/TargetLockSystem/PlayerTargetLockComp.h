// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "TargetLockComponent.h"
#include "PlayerTargetLockComp.generated.h"


class UTargetLockWidget;
class AAInGameCharacter;
class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;
class UCameraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UPlayerTargetLockComp : public UTargetLockComponent
{
	GENERATED_BODY()

public:
	UPlayerTargetLockComp();

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// player configurations start
	UPROPERTY()
	UInputComponent* PlayerInputComponent = nullptr;
	
	float OriginalMouseSensitivity = 1.0f;

	UPROPERTY()
	UCameraComponent* PlayerCamera = nullptr;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="TargetLock | Input")
	TObjectPtr<UInputAction> LockAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="TargetLock | Input")
	TObjectPtr<UInputAction> SwitchAction;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="TargetLock | Collision Debug")
	bool bDrawDetectDebug;
	// player configurations end

	virtual void DetectEnemies() override;

	virtual void UpdateCameraRotation(float DeltaTime) override;

	virtual void OnLockStateChanged(bool bNewLockState) override;
	
private:

	void BindLockOnInput();

	void OnSwitchTargetInput();

	UPROPERTY()
	AAInGameCharacter* PlayerPawn;

	UPROPERTY()
	APlayerController* PlayerController;

	UPROPERTY()
	UTargetLockWidget* LockWidget;
};
