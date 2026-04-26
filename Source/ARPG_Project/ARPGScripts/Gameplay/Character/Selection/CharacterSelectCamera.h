// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "CharacterSelectCamera.generated.h"

class UInputAction;
class AAInGameCharacter;

UCLASS()
class ARPG_PROJECT_API ACharacterSelectCamera : public ACameraActor
{
	GENERATED_BODY()

public:
	ACharacterSelectCamera();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

public:
	UPROPERTY()
	FVector DisplayLocation;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Character Selection")
	FVector DisplayLocationOffset;

	UPROPERTY()
	FRotator DisplayRotation;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Character Selection")
	FRotator DisplayRotationOffset;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Character Selection")
	float RolePreviewRotSpeed{0.5f};

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Character Selection")
	TObjectPtr<UAnimMontage> AnimAsset;

	UFUNCTION()
	void PreviewCharacterMesh(const FString& InCharacterID);

	//TODO:Implement Character Rotate in Tick
	void UpdatePreviewRot();
private:
	UPROPERTY()
	TObjectPtr<AAInGameCharacter> PreviewCharacter;

	FRotator TargetPreviewRot;
};
