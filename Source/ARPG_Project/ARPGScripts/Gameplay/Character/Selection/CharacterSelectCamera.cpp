// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterSelectCamera.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"


ACharacterSelectCamera::ACharacterSelectCamera()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACharacterSelectCamera::BeginPlay()
{
	Super::BeginPlay();

	DisplayLocation = GetActorLocation();
	DisplayRotation = GetActorRotation();
	//PreviewCharacterMesh(FString("00"));
}

void ACharacterSelectCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
}

void ACharacterSelectCamera::PreviewCharacterMesh(const FString& InCharacterID)
{
	//  change preview pawn mesh to play animation
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		FCharacterData PreviewCharData = GI->GetCharacterDataByID(InCharacterID);
		if (PreviewCharData.GetCharacterManifest())
		{
			if (TSubclassOf<APawn> PreviewCharClass = PreviewCharData.GetCharacterManifest()->CharacterClass)
			{
				FVector SpawnLoc = DisplayLocation + DisplayLocationOffset;
				FRotator SpawnRot = DisplayRotation + DisplayRotationOffset;
				if (!IsValid(PreviewCharacter))
				{
					PreviewCharacter = Cast<AAInGameCharacter>(GetWorld()->SpawnActor(PreviewCharClass, &SpawnLoc,&SpawnRot));
				}
				else
				{
					PreviewCharacter->Destroy();
					PreviewCharacter = Cast<AAInGameCharacter>(GetWorld()->SpawnActor(PreviewCharClass, &SpawnLoc,&SpawnRot));
				}
				PreviewCharacter->SetOwner(this);

				// play animation
				AnimAsset = PreviewCharData.GetCharacterManifest()->CharacterPreviewAnimAsset;
				if (IsValid(AnimAsset))
				{
					PreviewCharacter->PlayAnimMontage(AnimAsset);
				}
				//TODO:Destroy preview character when confirm
			}
		}
	}
	
	// refresh character info
	
}

void ACharacterSelectCamera::UpdatePreviewRot()
{

}



