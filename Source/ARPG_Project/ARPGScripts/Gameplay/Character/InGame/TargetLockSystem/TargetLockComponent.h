// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetLockComponent.generated.h"

USTRUCT(blueprintType)
struct FTargetLockConfig
{
	GENERATED_BODY()

	// 检测半径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float DetectRadius = 1000.0f;

	// 敌人检测图层
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	TEnumAsByte<ECollisionChannel> EnemyChannel = ECC_GameTraceChannel1;

	// 视野角度（仅检测前方敌人）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float FOVAngle = 60.0f;

	// 相机跟随速度（平滑看向目标）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float CameraFollowSpeed = 10.0f;

	// 相机Pitch轴限制（防止抬头/低头过度）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	FVector2D PitchLimit = FVector2D(-30.0f, 30.0f);
};


UCLASS(Abstract,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_PROJECT_API UTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetLockComponent();

	// basic members that use outside start
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	virtual void ToggleLockOn();
	
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SwitchLockTarget(bool bNext);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	FTargetLockConfig LockOnSettings;
	
	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	AActor* CurrentLockTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	AActor* LastLockTarget = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "LockOn")
	bool bIsLocking = false;
	// basic members that use outside end

protected:
	TArray<AActor*> CandidateEnemies;
	
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// implementable func for subclass
	virtual void DetectEnemies() {};
	
	virtual void UpdateCameraRotation(float DeltaTime) {};
	
	virtual void OnLockStateChanged(bool bNewLockState) {};
	// 

	// helper
	UFUNCTION()
	bool IsEnemyValid(AActor* Enemy);
	
	FRotator GetFocusTargetRot(AActor* Target, const FRotator& CurrentRot);
	
	void ClearCandidateEnemies();
	
	void CheckTargetValidity();
	
};
