// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGLeaf_ProgressBar.h"
#include "Components/ProgressBar.h"

void UARPGLeaf_ProgressBar::SetPercent(float Percent)
{
	TargetPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	if (IsValid(ProgressBar_LeafBar))
	{
		ProgressBar_LeafBar->SetPercent(TargetPercent);
	}

	// set timer call every 0.02 second
	if (!BufferedTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(
			BufferedTimerHandle,
			this,
			&UARPGLeaf_ProgressBar::UpdateBufferedPercent,
			0.02f,
			true);
	}
}

void UARPGLeaf_ProgressBar::UpdateBufferedPercent()
{
	if (!IsValid(ProgressBar_BufferedBar))
	{
		GetWorld()->GetTimerManager().ClearTimer(BufferedTimerHandle);
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	float NewBufferedPercent = FMath::FInterpTo(CurrentBufferedPercent, TargetPercent, DeltaTime, BufferedSpeed);
	ProgressBar_BufferedBar->SetPercent(NewBufferedPercent);
	CurrentBufferedPercent = NewBufferedPercent;

	if (FMath::IsNearlyEqual(CurrentBufferedPercent, TargetPercent, 0.001f))
	{
		GetWorld()->GetTimerManager().ClearTimer(BufferedTimerHandle);
		CurrentBufferedPercent = TargetPercent;
		ProgressBar_BufferedBar->SetPercent(TargetPercent);
	}
}
