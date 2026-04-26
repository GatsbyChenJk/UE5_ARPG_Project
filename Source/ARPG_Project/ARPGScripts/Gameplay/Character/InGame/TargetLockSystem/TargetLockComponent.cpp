// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetLockComponent.h"

#include "Kismet/KismetMathLibrary.h"


UTargetLockComponent::UTargetLockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTargetLockComponent::ToggleLockOn()
{
	GEngine->AddOnScreenDebugMessage(-1,.5f,FColor::Red,FString::Printf(TEXT("ToggleLock State:%d"),bIsLocking));
	if (bIsLocking)
	{
		CurrentLockTarget = nullptr;
		bIsLocking = false;
		ClearCandidateEnemies();
	}
	else
	{
		DetectEnemies();
		if (!CandidateEnemies.IsEmpty())
		{
			CurrentLockTarget = CandidateEnemies[0];
			bIsLocking = true;
			GEngine->AddOnScreenDebugMessage(-1,.5f,FColor::Red,FString::Printf(TEXT("Current Detect Enemy:%s"),*CurrentLockTarget->GetName()));
		}
	}
	
	OnLockStateChanged(bIsLocking);
}

void UTargetLockComponent::SwitchLockTarget(bool bNext)
{
	if (!bIsLocking || CandidateEnemies.IsEmpty()) return;

	int32 CurrentIndex = CandidateEnemies.IndexOfByKey(CurrentLockTarget);
	int32 NewIndex = bNext ? (CurrentIndex + 1) % CandidateEnemies.Num() : (CurrentIndex - 1);

	NewIndex = FMath::Clamp(NewIndex, 0, CandidateEnemies.Num() - 1);
	LastLockTarget = CurrentLockTarget;
	CurrentLockTarget = CandidateEnemies[NewIndex];
}


void UTargetLockComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTargetLockComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLocking)
	{
		CheckTargetValidity();
		UpdateCameraRotation(DeltaTime); // 子类实现相机控制
	}
}

bool UTargetLockComponent::IsEnemyValid(AActor* Enemy)
{
	if (!Enemy || Enemy->IsPendingKillPending()) return false;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return false;
	}
	
	FVector OwnerLocation = OwnerPawn->GetActorLocation();
	FVector EnemyLocation = Enemy->GetActorLocation();
	if (FVector::Dist2D(OwnerLocation,EnemyLocation) > 2.f * LockOnSettings.DetectRadius)
	{
		return false;
	}

	// TODO: if enemy is dead,then return false
	
	return true;
}

FRotator UTargetLockComponent::GetFocusTargetRot(AActor* Target, const FRotator& CurrentRot)
{
	if (!IsValid(Target)) return CurrentRot;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	FVector TargetLocation = Target->GetActorLocation();
	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(OwnerPawn->GetActorLocation(), TargetLocation);
	
	return TargetRot;
}

void UTargetLockComponent::ClearCandidateEnemies()
{
	CandidateEnemies.Empty();
}

void UTargetLockComponent::CheckTargetValidity()
{
	if (bIsLocking && !IsEnemyValid(CurrentLockTarget))
	{
		ToggleLockOn();
	}
}


