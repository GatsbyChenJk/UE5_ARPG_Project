 #pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "ItemSpawnPoint.generated.h"

/**
 * Item spawn configuration structure
 */
USTRUCT(BlueprintType)
struct FItemSpawnConfig
{
	GENERATED_BODY()

	/** The actor class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig")
	TSubclassOf<AActor> ItemClass;

	/** Number of items to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig", meta = (ClampMin = "1", ClampMax = "100"))
	int32 SpawnCount = 1;

	/** Radius within which to spawn items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig", meta = (ClampMin = "0"))
	float SpawnRadius = 300.0f;

	/** Minimum distance between spawned items to prevent overlap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig", meta = (ClampMin = "0"))
	float MinDistanceBetweenItems = 50.0f;

	/** Whether to randomize rotation for each spawned item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig")
	bool bRandomizeRotation = true;

	/** Whether to spawn items on ground (line trace down to find ground) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig")
	bool bSnapToGround = true;

	/** Height offset for line trace when finding ground */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig")
	float TraceHeight = 500.0f;

	/** Vertical offset from ground after spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig")
	float GroundOffset = 10.0f;

	/** Delay between each spawn (0 for instant) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig", meta = (ClampMin = "0"))
	float SpawnDelay = 0.0f;

	/** Whether to respawn items after they are destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig")
	bool bRespawnOnDestroy = false;

	/** Time to wait before respawning (if bRespawnOnDestroy is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | SpawnConfig", meta = (ClampMin = "0"))
	float RespawnDelay = 30.0f;
};

/**
 * Spawn point for items/loot in the game world.
 * Supports spawning multiple items with configurable positions to prevent overlap.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARPG_PROJECT_API AItemSpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	AItemSpawnPoint();

	/**
	 * Spawns all configured items
	 */
	UFUNCTION(BlueprintCallable, Category = "Item | Spawn")
	void SpawnItems();

	/**
	 * Spawns a single item at the specified index
	 * @param Index - The index of the item to spawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Item | Spawn")
	void SpawnItemAtIndex(int32 Index);

	/**
	 * Respawns a specific item after it was destroyed
	 * @param Index - The index of the item to respawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Item | Spawn")
	void RespawnItem(int32 Index);

	/**
	 * Destroys all spawned items
	 */
	UFUNCTION(BlueprintCallable, Category = "Item | Spawn")
	void DestroyAllItems();

	/**
	 * Gets all currently spawned items
	 */
	UFUNCTION(BlueprintPure, Category = "Item | Spawn")
	TArray<AActor*> GetSpawnedItems() const;

	/**
	 * Gets the number of currently spawned items
	 */
	UFUNCTION(BlueprintPure, Category = "Item | Spawn")
	int32 GetSpawnedItemCount() const { return SpawnedItems.Num(); }

	/**
	 * Checks if all items have been spawned
	 */
	UFUNCTION(BlueprintPure, Category = "Item | Spawn")
	bool AreAllItemsSpawned() const { return SpawnedItems.Num() >= ItemConfig.SpawnCount; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	/**
	 * Finds a valid spawn location that doesn't overlap with other items
	 * @param SpawnIndex - The index of the item being spawned
	 * @param MaxAttempts - Maximum number of attempts to find a valid location
	 * @return Valid spawn location, or ZeroVector if none found
	 */
	UFUNCTION(BlueprintCallable, Category = "Item | Spawn")
	FVector FindValidSpawnLocation(int32 SpawnIndex, int32 MaxAttempts = 20);

	/**
	 * Checks if a location is valid (not too close to other spawned items)
	 * @param Location - The location to check
	 * @return True if the location is valid
	 */
	UFUNCTION(BlueprintPure, Category = "Item | Spawn")
	bool IsLocationValid(const FVector& Location) const;

	/**
	 * Performs a line trace to find the ground at the given location
	 * @param Location - The starting location
	 * @return The ground location, or original location if no ground found
	 */
	UFUNCTION(BlueprintCallable, Category = "Item | Spawn")
	FVector FindGroundLocation(const FVector& Location) const;

	/**
	 * Handles item destruction for respawning
	 */
	UFUNCTION()
	void OnItemDestroyed(AActor* DestroyedActor);

	/**
	 * Schedules a respawn for a destroyed item
	 */
	void ScheduleRespawn(int32 Index);

public:
	/** Configuration for item spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Config")
	FItemSpawnConfig ItemConfig;

	/** Whether to spawn items automatically on begin play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Config")
	bool bSpawnOnBeginPlay = true;

private:
	/** Array of spawned items */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedItems;

	/** Cached spawn locations to prevent overlap */
	TArray<FVector> UsedSpawnLocations;

	/** Map of destroyed item indices to their respawn timers */
	TMap<int32, float> RespawnTimers;

	/** Timer handle for delayed spawning */
	FTimerHandle SpawnTimerHandle;

	/** Current spawn index for delayed spawning */
	int32 CurrentSpawnIndex = 0;
};