// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTargetLockComp.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "ARPGScripts/Widgets/Base/WidgetManager.h"
#include "ARPGScripts/Widgets/InGame/TargetLock/TargetLockWidget.h"
#include "Camera/CameraComponent.h"
#include "Engine/OverlapResult.h"



UPlayerTargetLockComp::UPlayerTargetLockComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UPlayerTargetLockComp::BeginPlay()
{
	Super::BeginPlay();

	PlayerPawn = Cast<AAInGameCharacter>(GetOwner());
	PlayerController = Cast<APlayerController>(PlayerPawn->GetController());
	if (IsValid(PlayerPawn) && IsValid(PlayerController))
	{
		PlayerCamera = PlayerPawn->FindComponentByClass<UCameraComponent>();
		PlayerInputComponent = PlayerController->InputComponent;
		BindLockOnInput();
	}
	
}


void UPlayerTargetLockComp::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UPlayerTargetLockComp::DetectEnemies()
{
	ClearCandidateEnemies();
	if (!IsValid(PlayerPawn)) return;
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerPawn);

	GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		PlayerPawn->GetActorLocation(),
		FQuat::Identity,
		LockOnSettings.EnemyChannel,
		FCollisionShape::MakeSphere(LockOnSettings.DetectRadius),
		QueryParams
	);
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* Enemy = Result.GetActor();
		if (IsEnemyValid(Enemy))
		{
			CandidateEnemies.Add(Enemy);
		}
	}

#if WITH_EDITOR
	if (bDrawDetectDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			PlayerPawn->GetActorLocation(),
			LockOnSettings.DetectRadius,
			12, // 细分段数
			FColor::Red,
			false, // 是否持续
			2.0f,  // 持续时间
			0,     // 深度优先级
			1.0f   // 线粗
		);
	}
#endif
	//TODO:Add other player to ignore actor
}

void UPlayerTargetLockComp::UpdateCameraRotation(float DeltaTime)
{
	if (!PlayerCamera || !CurrentLockTarget) return;
	
	if (!PlayerController) return;
	
	FRotator NewRot = GetFocusTargetRot(CurrentLockTarget, PlayerCamera->GetComponentRotation());
	
	FRotator ControlRot = PlayerController->GetControlRotation();
	ControlRot.Yaw = NewRot.Yaw;
	PlayerController->SetControlRotation(ControlRot);
}

void UPlayerTargetLockComp::OnLockStateChanged(bool bNewLockState)
{
	if (!IsValid(PlayerController)) return;

	if (!IsValid(LockWidget))
	{
		if (const auto WidgetMgr = GetWorld()->GetGameInstance()->GetSubsystem<UWidgetManager>())
		{
			GEngine->AddOnScreenDebugMessage(-1,1.5f,FColor::Red,
				FString::Printf(TEXT("LockWidget add on role:%d"),GetOwner()->GetLocalRole()));
			LockWidget = Cast<UTargetLockWidget>(WidgetMgr->GetOpenedUIByID(FString("1006")));
		}
	}

	if (bNewLockState)
	{
		LockWidget->AttachedActor = CurrentLockTarget;
		LockWidget->ToggleWidget(PlayerController);
	}
	else
	{
		LockWidget->AttachedActor = nullptr;
		LockWidget->ToggleWidget(PlayerController);
	}
}

void UPlayerTargetLockComp::BindLockOnInput()
{
	if (!PlayerInputComponent) return;

	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//InputSubSystem =  ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		
		EnhancedInputComp->BindAction(LockAction, ETriggerEvent::Triggered, this, &ThisClass::ToggleLockOn);
		EnhancedInputComp->BindAction(SwitchAction,ETriggerEvent::Triggered,this,&ThisClass::OnSwitchTargetInput);
	}
	
}

void UPlayerTargetLockComp::OnSwitchTargetInput()
{
	bool bGetNextTarget = true;
	SwitchLockTarget(bGetNextTarget);

	if (IsValid(LockWidget))
	{
		LockWidget->AttachedActor = CurrentLockTarget;
	}
	
}

