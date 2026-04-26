// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Base/BaseWidget.h"
#include "TargetLockWidget.generated.h"

class USizeBox;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UTargetLockWidget : public UBaseWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
    AActor* AttachedActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldOffset;

protected:
	virtual void OnOpenWidget(APlayerController* OwningController) override;

	virtual void OnCloseWidget(APlayerController* OwningController) override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	UPROPERTY(meta=(BindWidget))
	USizeBox* ParentSizeBox;
	
};
