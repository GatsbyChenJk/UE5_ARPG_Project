#include "ItemSpawnPoint.h"
#include "ARPGScripts/Gameplay/Base/ARPGObjectPoolSystem/PoolSubsystem.h"

AItemSpawnPoint::AItemSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AItemSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bSpawnOnBeginPlay)
	{
		if (ItemConfig.SpawnDelay > 0.0f)
		{
			// Start delayed spawning
			CurrentSpawnIndex = 0;
			GetWorldTimerManager().SetTimer(
				SpawnTimerHandle,
				[this]()
				{
					SpawnItemAtIndex(CurrentSpawnIndex++);
					if (CurrentSpawnIndex >= ItemConfig.SpawnCount)
					{
						GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
					}
				},
				ItemConfig.SpawnDelay,
				true
			);
		}
		else
		{
			SpawnItems();
		}
	}
}

void AItemSpawnPoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	// Clean up spawned items
	DestroyAllItems();
}

void AItemSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle respawn timers
	if (HasAuthority() && ItemConfig.bRespawnOnDestroy && RespawnTimers.Num() > 0)
	{
		TArray<int32> IndicesToRespawn;

		for (auto& Pair : RespawnTimers)
		{
			Pair.Value -= DeltaTime;
			if (Pair.Value <= 0.0f)
			{
				IndicesToRespawn.Add(Pair.Key);
			}
		}

		for (int32 Index : IndicesToRespawn)
		{
			RespawnTimers.Remove(Index);
			RespawnItem(Index);
		}
	}
}

void AItemSpawnPoint::SpawnItems()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(ItemConfig.ItemClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemSpawnPoint: No item class specified in config"));
		return;
	}

	// Pre-allocate arrays
	SpawnedItems.Reserve(ItemConfig.SpawnCount);
	UsedSpawnLocations.Reserve(ItemConfig.SpawnCount);

	for (int32 i = 0; i < ItemConfig.SpawnCount; ++i)
	{
		SpawnItemAtIndex(i);
	}
}

void AItemSpawnPoint::SpawnItemAtIndex(int32 Index)
{
	if (!HasAuthority() || !IsValid(ItemConfig.ItemClass))
	{
		return;
	}

	// Check if item already exists at this index
	if (SpawnedItems.IsValidIndex(Index) && IsValid(SpawnedItems[Index]))
	{
		return;
	}

	FVector SpawnLocation = FindValidSpawnLocation(Index);
	if (SpawnLocation == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemSpawnPoint: Failed to find valid spawn location for item %d"), Index);
		return;
	}

	UsedSpawnLocations.Add(SpawnLocation);

	FRotator SpawnRotation = ItemConfig.bRandomizeRotation
		? FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f)
		: GetActorRotation();

	FTransform SpawnTransform(SpawnRotation, SpawnLocation);
	AActor* NewItem = nullptr;
	if (UPoolSubsystem* PoolSub = GetGameInstance()->GetSubsystem<UPoolSubsystem>())
	{
		NewItem = PoolSub->RequestActor(this, ItemConfig.ItemClass, SpawnTransform);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		NewItem = GetWorld()->SpawnActor(ItemConfig.ItemClass, &SpawnLocation, &SpawnRotation, SpawnParams);
	}

	if (NewItem)
	{
		// Ensure we have enough space in the array
		if (SpawnedItems.Num() <= Index)
		{
			SpawnedItems.SetNum(Index + 1);
		}
		SpawnedItems[Index] = NewItem;

		// Bind to destruction event if respawning is enabled
		if (ItemConfig.bRespawnOnDestroy)
		{
			NewItem->OnDestroyed.AddDynamic(this, &AItemSpawnPoint::OnItemDestroyed);
		}
	}
}

void AItemSpawnPoint::RespawnItem(int32 Index)
{
	if (!HasAuthority())
	{
		return;
	}

	// Remove the old location from used locations if valid
	if (UsedSpawnLocations.IsValidIndex(Index))
	{
		UsedSpawnLocations.RemoveAt(Index);
	}

	SpawnItemAtIndex(Index);
}

void AItemSpawnPoint::DestroyAllItems()
{
	UPoolSubsystem* PoolSub = GetGameInstance()->GetSubsystem<UPoolSubsystem>();
	for (auto& Item : SpawnedItems)
	{
		if (IsValid(Item))
		{
			Item->OnDestroyed.RemoveDynamic(this, &AItemSpawnPoint::OnItemDestroyed);
			if (PoolSub)
			{
				PoolSub->ReleaseActor(this, Item);
			}
			else
			{
				Item->Destroy();
			}
		}
	}
	SpawnedItems.Empty();
	UsedSpawnLocations.Empty();
	RespawnTimers.Empty();
}

TArray<AActor*> AItemSpawnPoint::GetSpawnedItems() const
{
	TArray<AActor*> ValidItems;
	for (const auto& Item : SpawnedItems)
	{
		if (IsValid(Item))
		{
			ValidItems.Add(Item);
		}
	}
	return ValidItems;
}

FVector AItemSpawnPoint::FindValidSpawnLocation(int32 SpawnIndex, int32 MaxAttempts)
{
	FVector BaseLocation = GetActorLocation();

	// First item always spawns at center
	if (SpawnIndex == 0 || ItemConfig.SpawnRadius <= 0.0f)
	{
		FVector Location = BaseLocation;
		if (ItemConfig.bSnapToGround)
		{
			Location = FindGroundLocation(Location);
		}
		return Location;
	}

	// Try to find a valid location using random distribution
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Generate random point within circle using uniform distribution
		float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
		float RandomRadius = FMath::Sqrt(FMath::RandRange(0.0f, 1.0f)) * ItemConfig.SpawnRadius;

		FVector RandomOffset(
			FMath::Cos(RandomAngle) * RandomRadius,
			FMath::Sin(RandomAngle) * RandomRadius,
			0.0f
		);

		FVector TestLocation = BaseLocation + RandomOffset;

		if (ItemConfig.bSnapToGround)
		{
			TestLocation = FindGroundLocation(TestLocation);
		}

		// Check if location is valid (not too close to other spawned items)
		if (IsLocationValid(TestLocation))
		{
			return TestLocation;
		}
	}

	// Fallback: Use grid-based placement for more uniform distribution
	int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(ItemConfig.SpawnCount)));
	float CellSize = (ItemConfig.SpawnRadius * 2.0f) / GridSize;

	int32 GridX = SpawnIndex % GridSize;
	int32 GridY = SpawnIndex / GridSize;

	// Add some randomness within the cell
	float CellRandomX = FMath::RandRange(-CellSize * 0.3f, CellSize * 0.3f);
	float CellRandomY = FMath::RandRange(-CellSize * 0.3f, CellSize * 0.3f);

	FVector GridOffset(
		(GridX - GridSize / 2.0f) * CellSize + CellSize / 2.0f + CellRandomX,
		(GridY - GridSize / 2.0f) * CellSize + CellSize / 2.0f + CellRandomY,
		0.0f
	);

	FVector GridLocation = BaseLocation + GridOffset;

	if (ItemConfig.bSnapToGround)
	{
		GridLocation = FindGroundLocation(GridLocation);
	}

	return GridLocation;
}

bool AItemSpawnPoint::IsLocationValid(const FVector& Location) const
{
	float MinDistSq = FMath::Square(ItemConfig.MinDistanceBetweenItems);

	for (const FVector& UsedLocation : UsedSpawnLocations)
	{
		if (FVector::DistSquared(Location, UsedLocation) < MinDistSq)
		{
			return false;
		}
	}
	return true;
}

FVector AItemSpawnPoint::FindGroundLocation(const FVector& Location) const
{
	FHitResult HitResult;
	FVector Start = Location + FVector(0.0f, 0.0f, ItemConfig.TraceHeight);
	FVector End = Location - FVector(0.0f, 0.0f, ItemConfig.TraceHeight);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		return HitResult.Location + FVector(0.0f, 0.0f, ItemConfig.GroundOffset);
	}

	return Location;
}

void AItemSpawnPoint::OnItemDestroyed(AActor* DestroyedActor)
{
	if (!HasAuthority() || !ItemConfig.bRespawnOnDestroy)
	{
		return;
	}

	// Find the index of the destroyed item
	int32 Index = SpawnedItems.IndexOfByKey(DestroyedActor);
	if (Index != INDEX_NONE)
	{
		SpawnedItems[Index] = nullptr;
		ScheduleRespawn(Index);
	}
}

void AItemSpawnPoint::ScheduleRespawn(int32 Index)
{
	if (ItemConfig.RespawnDelay > 0.0f)
	{
		RespawnTimers.Add(Index, ItemConfig.RespawnDelay);
	}
	else
	{
		RespawnItem(Index);
	}
}
