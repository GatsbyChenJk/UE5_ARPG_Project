// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "FARPGEventData.h"
#include "ARPGEventManager.generated.h"

ARPG_PROJECT_API DECLARE_LOG_CATEGORY_EXTERN(LogARPGEvent, Log, All);

UENUM(BlueprintType)
enum class EARPGEventPriority : uint8
{
	Low = 0,
	Normal = 1,
	High = 2,
	Critical = 3
};

struct FARPGObjectListener
{
	TWeakObjectPtr<UObject> Object;
	UFunction* Function;
	EARPGEventPriority Priority;

	FARPGObjectListener()
		: Object(nullptr), Function(nullptr), Priority(EARPGEventPriority::Normal) {}

	FARPGObjectListener(UObject* InObject, UFunction* InFunction, EARPGEventPriority InPriority = EARPGEventPriority::Normal)
		: Object(InObject), Function(InFunction), Priority(InPriority) {}

	bool operator<(const FARPGObjectListener& Other) const
	{
		return static_cast<uint8>(Priority) > static_cast<uint8>(Other.Priority);
	}
};

struct FARPGPendingEvent
{
	FName EventName;
	TObjectPtr<UARPGEventData> EventData;
	int64 Timestamp;

	FARPGPendingEvent() = default;
	FARPGPendingEvent(const FName& InName, UARPGEventData* InData)
		: EventName(InName), EventData(InData), Timestamp(FDateTime::UtcNow().GetTicks()) {}
};

USTRUCT(BlueprintType)
struct FARPGEventStats
{
	GENERATED_BODY()
	int32 TotalDispatched = 0;
	int32 QueueMaxSize = 0;
	int32 ListenersCount = 0;
};

USTRUCT(BlueprintType)
struct FARPGEventManagerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableQueue = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxEventsPerFrame = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bProcessWhenPaused = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 QueueWarningThreshold = 1000;
};

UCLASS(Blueprintable, BlueprintType)
class ARPG_PROJECT_API UARPGEventManager : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	bool RegisterARPGEventObject(UObject* ListenerObject, const FName& EventName, EARPGEventPriority Priority = EARPGEventPriority::Normal);

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void UnregisterARPGEventObject(UObject* ListenerObject, const FName& EventName);

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void UnregisterAllARPGEventsForActor(AActor* ListenerActor);

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void UnregisterAllARPGEventsForObject(UObject* ListenerObject);

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void DispatchARPGEvent(const FName& EventName, UARPGEventData* EventData);

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void DispatchUObjectARPGEvent(const FName& EventName, UARPGEventData* EventData);

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void DispatchARPGEventToTarget(AActor* TargetActor, const FName& EventName, UARPGEventData* EventData);

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void DispatchARPGEventToUObjectTarget(UObject* TargetObject, const FName& EventName, UARPGEventData* EventData);

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void FlushEventQueue();

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void ClearEventQueue();

	UFUNCTION(BlueprintPure, Category = "ARPG|Event")
	int32 GetQueueSize() const { return EventQueue.Num(); }

	UFUNCTION(BlueprintPure, Category = "ARPG|Event")
	int32 GetListenerCount(const FName& EventName) const;

	UFUNCTION(BlueprintPure, Category = "ARPG|Event")
	const FARPGEventManagerConfig& GetConfig() const { return Config; }

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void SetConfig(const FARPGEventManagerConfig& InConfig) { Config = InConfig; }

	UFUNCTION(BlueprintPure, Category = "ARPG|Event")
	const FARPGEventStats& GetStats() const { return Stats; }

	UFUNCTION(BlueprintCallable, Category = "ARPG|Event")
	void ResetStats();

private:
	TMap<FName, TArray<FARPGObjectListener>> EventListenerMap;
	TArray<FARPGPendingEvent> EventQueue;

	UPROPERTY()
	TArray<TObjectPtr<UARPGEventData>> QueueDataGuard;

	UPROPERTY(EditAnywhere, Category = "Config")
	FARPGEventManagerConfig Config;

	UPROPERTY(Transient)
	FARPGEventStats Stats;

	bool bIsProcessingQueue = false;

	void ConsumeEvent(const FARPGPendingEvent& PendingEvent);
	void DispatchToListeners(const FName& EventName, UARPGEventData* EventData);
	void CleanupInvalidListeners(const FName& EventName);

	FName GetARPGListenerFuncName(const FName& EventName) const
	{
		return *FString::Printf(TEXT("ARPG_%s"), *EventName.ToString());
	}

	bool IsValidARPGListenerFunction(UFunction* Func) const;
};
