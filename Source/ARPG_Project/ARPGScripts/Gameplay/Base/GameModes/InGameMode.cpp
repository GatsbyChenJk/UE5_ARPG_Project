// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMode.h"

#include "TimerManager.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "ARPGScripts/Gameplay/Base/SaveGameSystem/ARPGSaveGameManager.h"
#include "ARPGScripts/Gameplay/Character/AI/AISpawnPoint.h"
#include "ARPGScripts/Gameplay/Combat/AInGamePlayerState.h"
#include "ARPGScripts/Gameplay/GameMap/EscapePointGenerator.h"
#include "ARPGScripts/Gameplay/GameMap/MapLoadingManager.h"
#include "ARPGScripts/Gameplay/GameMap/MapPointManager.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterAttackComponent.h"
#include "ARPGScripts/Gameplay/Weapon/CharacterShieldComponent.h"
#include "ARPGScripts/Gameplay/Weapon/WeaponBase/ARPGBaseWeapon.h"


void AInGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
}

void AInGameMode::BeginPlay()
{
	Super::BeginPlay();
}

APawn* AInGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	TSubclassOf<APawn> PawnClassToSpawn = GetPlayerPawnClassFromGameInstance(NewPlayer);
	if (PawnClassToSpawn == nullptr)
	{
		// use thorn(thirdperson) character as default pawn
		UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance());
		FCharacterData CharacterData = GameInstance->GetCharacterDataByID(FString("000"));
		if (CharacterData.GetCharacterManifest())
		{
			PawnClassToSpawn = CharacterData.GetCharacterManifest()->CharacterClass;
		}
		else
		{
			PawnClassToSpawn = DefaultPawnClass;
		}
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.ObjectFlags |= RF_Transient;

	FTransform CharaterSpawnPos = GetPlayerPawnTransformFromGameInstance(NewPlayer);
	APawn* SpawnedPawn;
	if (!CharaterSpawnPos.GetLocation().Equals(FVector::ZeroVector))
	{
		SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClassToSpawn, CharaterSpawnPos, SpawnParams);
	}
	else
	{
		SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClassToSpawn, SpawnTransform, SpawnParams);
	}
	

	// server init player attributes
	if (SpawnedPawn && HasAuthority()) 
	{
		NewPlayer->Possess(SpawnedPawn);
		AAInGamePlayerState* PS = NewPlayer->GetPlayerState<AAInGamePlayerState>();
		if (PS)
		{
			PS->InitializeASCForPlayer(NewPlayer, SpawnedPawn);
		}
	}

	// TODO:find a way to send event
	// or call inventory func to attach weapon
	
	return SpawnedPawn;
}

void AInGameMode::RequestRespawn(AController* Controller, float Delay)
{
	if (!IsValid(Controller)) return;

	if (AAInGamePlayerState* PS = Controller->GetPlayerState<AAInGamePlayerState>())
	{
		PS->ResetPlayer();
	}

	if (auto BaseChar = Controller->GetPawn())
	{
		if (auto ARPGChar = Cast<AARPGBaseCharacter>(Controller->GetPawn()))
		{
			if (const auto AttackComp = ARPGChar->GetCharacterAttackComponent())
			{
				if (auto EquippedWeapon = AttackComp->GetCurrentEquippedWeapon())
				{
					EquippedWeapon->Destroy();
				}
			}

			if (const auto DefenseComp = ARPGChar->GetCharacterShieldComponent())
			{
				if (auto EquippedWeapon = DefenseComp->GetCurrentEquippedWeapon())
				{
					EquippedWeapon->Destroy();
				}
			}	
		}
		
		BaseChar->Destroy();
	}

	if (Delay > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &AInGameMode::PerformRespawn, Controller);
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, Delegate, Delay, false);
	}
	else
	{
		PerformRespawn(Controller);
	}
}

void AInGameMode::RegisterAISpawnPoint_Implementation(AAISpawnPoint* SpawnPoint)
{
	if (IsValid(SpawnPoint) && !AISpawnPoints.Contains(SpawnPoint))
	{
		AISpawnPoints.Add(SpawnPoint);
	}
}

void AInGameMode::UnRegisterAISpawnPoint_Implementation(AAISpawnPoint* SpawnPoint)
{
	if (IsValid(SpawnPoint) && AISpawnPoints.Contains(SpawnPoint))
	{
		AISpawnPoints.Remove(SpawnPoint);
	}
}

void AInGameMode::Server_ReturnToLobby_Implementation()
{
	if (!HasAuthority()) return;
	if (UMapLoadingManager* MapLoader = GetGameInstance()->GetSubsystem<UMapLoadingManager>())
	{
		MapLoader->ServerTravelToMapByID(LobbyMapID);
	}
}

TSubclassOf<APawn> AInGameMode::GetPlayerPawnClassFromGameInstance(AController* NewPlayer)
{
	if (NewPlayer)
	{
		if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			FCharacterData CharData = GameInstance->GetCharacterData();
			if (CharData.bIsCharacterSet && CharData.GetCharacterManifest()->CharacterClass)
			{
				return CharData.GetCharacterManifest()->CharacterClass;
			}
		}
	}
	return nullptr;
}

FTransform AInGameMode::GetPlayerPawnTransformFromGameInstance(AController* NewPlayer)
{
	if (IsValid(NewPlayer))
	{
		if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
		{
			
			FMapManifest Manifest = GameInstance->GetMapManifestByID(IngameMapID);
			if(Manifest.SpawnPoints.Num() > 0)
			{
				if (auto MapManager = GameInstance->MapManager)
				{
					MapManager->InitPointManager(Manifest);
					FTransform FinalTransform = MapManager->GetRandomSpawnPointTransform();
					return FinalTransform;
				}
			}
			
		}
	}
	return FTransform();
}

void AInGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	SpawnPortals();

	for (auto SpawnPoint : AISpawnPoints)
	{
		SpawnPoint->SpawnBasicActors();
	}
}

void AInGameMode::SpawnPortals()
{
	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
	{
		FMapManifest Manifest = GameInstance->GetMapManifestByID(IngameMapID);

		// Detailed diagnostics for M06 issue
		UE_LOG(LogTemp, Log, TEXT("SpawnPortals: Querying MapID='%s', Result MapID='%s', LevelPath='%s'"),
			*IngameMapID, *Manifest.MapID, *Manifest.GetLevelPath());
		UE_LOG(LogTemp, Log, TEXT("SpawnPortals: EscapePoints count=%d, SpawnPoints count=%d"),
			Manifest.EscapePoints.Num(), Manifest.SpawnPoints.Num());

		// Log details of each EscapePoint entry
		for (int32 i = 0; i < Manifest.EscapePoints.Num(); ++i)
		{
			const FEscapePointData& Point = Manifest.EscapePoints[i];
			UE_LOG(LogTemp, Log, TEXT("SpawnPortals: EscapePoint[%d] - bIsEnabled=%s, PortalClass=%s, Location=%s"),
				i,
				Point.bIsEnabled ? TEXT("true") : TEXT("false"),
				Point.PortalClass ? *Point.PortalClass->GetName() : TEXT("NULL"),
				*Point.PointTransform.GetLocation().ToString());
		}

		if (auto MapManager = GameInstance->MapManager)
		{
			if (Manifest.EscapePoints.Num() > 0)
			{
				MapManager->InitPointManager(Manifest);
				FTransform PortalTransform = MapManager->GetRandomEscapePointTransform();
				if (auto ActorClass = MapManager->GetEscapePointGenerator()->GetEscapePointActor())
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
					SpawnParams.ObjectFlags |= RF_Transient;
					SpawnParams.Owner = this;

					AActor* PortalActor = GetWorld()->SpawnActor(ActorClass,&PortalTransform,SpawnParams);
					if (PortalActor)
					{
						UE_LOG(LogTemp, Log, TEXT("SpawnPortals: Successfully spawned portal at %s"), *PortalTransform.GetLocation().ToString());
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("SpawnPortals: SpawnActor returned NULL - check collision or spawn location!"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("SpawnPortals: EscapePointGenerator returned null ActorClass for MapID %s"), *IngameMapID);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SpawnPortals: No EscapePoints found for MapID '%s'. Check if IngameMapID is correctly set in GameMode blueprint."), *IngameMapID);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SpawnPortals: MapManager is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnPortals: Failed to get PlayerGameInstance"));
	}
}

void AInGameMode::PerformRespawn(AController* Controller)
{
	FTransform SpawnTransform = GetPlayerPawnTransformFromGameInstance(Controller);
	if (SpawnTransform.IsValid())
	{
		RestartPlayerAtTransform(Controller, SpawnTransform);
	}
	else
	{
		RestartPlayer(Controller);
	}
}
