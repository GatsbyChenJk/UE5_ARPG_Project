// Fill out your copyright notice in the Description page of Project Settings.


#include "AISpawnPoint.h"

#include "ARPGScripts/Gameplay/AIController/InGameAIController.h"
#include "ARPGScripts/Gameplay/Base/GameModes/InGameMode.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameAICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"


AAISpawnPoint::AAISpawnPoint()
{
	bReplicates = true;
	SpawnCount = 1;
	SpawnRadius = 500.0f;
	MinDistanceBetweenAI = 200.0f;
	TraceHeight = 1000.0f;
	bRandomizeRotation = false;
}

void AAISpawnPoint::SpawnBasicActors()
{
	if (!HasAuthority())
	{
		return;
	}

	SpawnMultipleAI();
}

void AAISpawnPoint::SpawnMultipleAI()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(AISpawnConfig.AIPawnClass) || !IsValid(AISpawnConfig.AIControllerClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("AISpawnPoint: Missing pawn or controller class in config"));
		return;
	}

	// Pre-allocate arrays
	SpawnedAICharacters.Reserve(SpawnCount);
	SpawnedAIControllers.Reserve(SpawnCount);
	UsedSpawnLocations.Reserve(SpawnCount);

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		FVector SpawnLocation = FindValidSpawnLocation(i);
		if (SpawnLocation == FVector::ZeroVector)
		{
			UE_LOG(LogTemp, Warning, TEXT("AISpawnPoint: Failed to find valid spawn location for AI %d"), i);
			continue;
		}

		UsedSpawnLocations.Add(SpawnLocation);

		FRotator SpawnRotation = bRandomizeRotation
			? FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f)
			: GetActorRotation();

		// Spawn AI Character
		AInGameAICharacter* NewAICharacter = nullptr;
		if (IsValid(AISpawnConfig.AIPawnClass))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			NewAICharacter = Cast<AInGameAICharacter>(GetWorld()->SpawnActor(
				AISpawnConfig.AIPawnClass,
				&SpawnLocation,
				&SpawnRotation,
				SpawnParams));

			if (NewAICharacter)
			{
				SpawnedAICharacters.Add(NewAICharacter);
			}
		}

		// Spawn AI Controller
		AInGameAIController* NewAIController = nullptr;
		if (IsValid(AISpawnConfig.AIControllerClass))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			NewAIController = Cast<AInGameAIController>(GetWorld()->SpawnActor(
				AISpawnConfig.AIControllerClass,
				&SpawnLocation,
				&SpawnRotation,
				SpawnParams));

			if (NewAIController)
			{
				SpawnedAIControllers.Add(NewAIController);
			}
		}

		// Spawn patrol path for this AI
		AActor* NewPatrolPath = nullptr;
		if (IsValid(AISpawnConfig.PatrollingPath))
		{
			NewPatrolPath = GetWorld()->SpawnActor(AISpawnConfig.PatrollingPath, &SpawnLocation, &SpawnRotation);
			if (NewPatrolPath)
			{
				SpawnedPatrolPaths.Add(NewPatrolPath);
			}
		}
	}

	// Initialize all spawned AI
	InitializeAI();
}

FVector AAISpawnPoint::FindValidSpawnLocation(int32 SpawnIndex, int32 MaxAttempts)
{
	FVector BaseLocation = GetActorLocation();

	// First AI always spawns at center
	if (SpawnIndex == 0 || SpawnRadius <= 0.0f)
	{
		return BaseLocation;
	}

	// Try to find a valid location
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Generate random point within circle
		float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
		float RandomRadius = FMath::Sqrt(FMath::RandRange(0.0f, 1.0f)) * SpawnRadius;

		FVector RandomOffset(
			FMath::Cos(RandomAngle) * RandomRadius,
			FMath::Sin(RandomAngle) * RandomRadius,
			0.0f
		);

		FVector TestLocation = BaseLocation + RandomOffset;

		// Line trace to find ground
		FHitResult HitResult;
		FVector Start = TestLocation + FVector(0.0f, 0.0f, TraceHeight);
		FVector End = TestLocation - FVector(0.0f, 0.0f, TraceHeight);

		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
		{
			TestLocation = HitResult.Location;
		}

		// Check if location is valid (not too close to other spawned AI)
		if (IsLocationValid(TestLocation))
		{
			return TestLocation;
		}
	}

	// Fallback: try grid-based placement
	if (SpawnIndex > 0)
	{
		int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(SpawnCount)));
		float CellSize = (SpawnRadius * 2.0f) / GridSize;

		int32 GridX = SpawnIndex % GridSize;
		int32 GridY = SpawnIndex / GridSize;

		FVector GridOffset(
			(GridX - GridSize / 2.0f) * CellSize + CellSize / 2.0f,
			(GridY - GridSize / 2.0f) * CellSize + CellSize / 2.0f,
			0.0f
		);

		FVector GridLocation = BaseLocation + GridOffset;

		// Line trace to find ground
		FHitResult HitResult;
		FVector Start = GridLocation + FVector(0.0f, 0.0f, TraceHeight);
		FVector End = GridLocation - FVector(0.0f, 0.0f, TraceHeight);

		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
		{
			GridLocation = HitResult.Location;
		}

		if (IsLocationValid(GridLocation))
		{
			return GridLocation;
		}
	}

	// Final fallback: return base location with slight random offset
	return BaseLocation + FVector(
		FMath::RandRange(-100.0f, 100.0f),
		FMath::RandRange(-100.0f, 100.0f),
		0.0f
	);
}

bool AAISpawnPoint::IsLocationValid(const FVector& Location) const
{
	for (const FVector& UsedLocation : UsedSpawnLocations)
	{
		if (FVector::DistSquared(Location, UsedLocation) < FMath::Square(MinDistanceBetweenAI))
		{
			return false;
		}
	}
	return true;
}

void AAISpawnPoint::InitializeAI()
{
	int32 ControllerCount = SpawnedAIControllers.Num();
	int32 CharacterCount = SpawnedAICharacters.Num();
	int32 MinCount = FMath::Min(ControllerCount, CharacterCount);

	for (int32 i = 0; i < MinCount; ++i)
	{
		InitializeAIAtIndex(i);
	}
}

void AAISpawnPoint::InitializeAIAtIndex(int32 Index)
{
	if (!SpawnedAICharacters.IsValidIndex(Index) || !SpawnedAIControllers.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("AISpawnPoint: Invalid index %d for initialization. Characters:%d, Controllers:%d"),
			Index, SpawnedAICharacters.Num(), SpawnedAIControllers.Num());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AISpawnPoint: Initializing AI at index %d"), Index);
	
	InitializeAIController(Index);

	if (SpawnedAIControllers[Index] && SpawnedAICharacters[Index])
	{
		UE_LOG(LogTemp, Log, TEXT("AISpawnPoint: Possessing character %s with controller %s"),
			*SpawnedAICharacters[Index]->GetName(), *SpawnedAIControllers[Index]->GetName());
		SpawnedAIControllers[Index]->Possess(SpawnedAICharacters[Index]);
		InitializeAICharacter(Index);
	}
}

void AAISpawnPoint::InitializeAICharacter(int32 Index)
{
	if (SpawnedAICharacters.IsValidIndex(Index) && SpawnedAICharacters[Index])
	{
		SpawnedAICharacters[Index]->AIBasicDataInitialize(AISpawnConfig);
	}
}

void AAISpawnPoint::InitializeAIController(int32 Index)
{
	if (!SpawnedAIControllers.IsValidIndex(Index) || !SpawnedAIControllers[Index])
	{
		UE_LOG(LogTemp, Warning, TEXT("AISpawnPoint: Invalid controller at index %d"), Index);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AISpawnPoint: Initializing AIController[%d] with BT=%s, BB=%s"),
		Index,
		AISpawnConfig.AITree ? *AISpawnConfig.AITree->GetName() : TEXT("NULL"),
		AISpawnConfig.AIBlackboard ? *AISpawnConfig.AIBlackboard->GetName() : TEXT("NULL"));

	SpawnedAIControllers[Index]->InitBasicAIConfig(AISpawnConfig);

	// Assign the corresponding patrol path
	if (SpawnedPatrolPaths.IsValidIndex(Index) && IsValid(SpawnedPatrolPaths[Index]))
	{
		SpawnedAIControllers[Index]->SetPatrolPath(SpawnedPatrolPaths[Index]);
	}
	// Fallback to shared patrol path
	else if (IsValid(AIPatrollingPath))
	{
		SpawnedAIControllers[Index]->SetPatrolPath(AIPatrollingPath);
	}
}

void AAISpawnPoint::ClientSpawnCharacter_Implementation(int32 Index)
{
	if (!SpawnedAICharacters.IsValidIndex(Index) || !IsValid(AISpawnConfig.AIPawnClass))
	{
		return;
	}

	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	SpawnedAICharacters[Index] = Cast<AInGameAICharacter>(GetWorld()->SpawnActor(
		AISpawnConfig.AIPawnClass,
		&SpawnLocation,
		&SpawnRotation));
}

void AAISpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (const auto GameMode = Cast<AInGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->RegisterAISpawnPoint(this);
		}
	}
}

void AAISpawnPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (HasAuthority())
	{
		if (const auto GameMode = Cast<AInGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->UnRegisterAISpawnPoint(this);
		}
	}

	// Clean up spawned actors
	for (auto& Character : SpawnedAICharacters)
	{
		if (IsValid(Character))
		{
			Character->Destroy();
		}
	}
	SpawnedAICharacters.Empty();

	for (auto& Controller : SpawnedAIControllers)
	{
		if (IsValid(Controller))
		{
			Controller->Destroy();
		}
	}
	SpawnedAIControllers.Empty();

	for (auto& PatrolPath : SpawnedPatrolPaths)
	{
		if (IsValid(PatrolPath))
		{
			PatrolPath->Destroy();
		}
	}
	SpawnedPatrolPaths.Empty();
}
