// Fill out your copyright notice in the Description page of Project Settings.

#include "ARPGEventManager.h"

DEFINE_LOG_CATEGORY(LogARPGEvent);

void UARPGEventManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UARPGEventManager::Deinitialize()
{
	ClearEventQueue();
	EventListenerMap.Empty();
	Super::Deinitialize();
}

void UARPGEventManager::Tick(float DeltaTime)
{
	if (!Config.bProcessWhenPaused)
	{
		if (UWorld* World = GetWorld())
		{
			if (World->IsPaused())
			{
				return;
			}
		}
	}
	FlushEventQueue();
}

bool UARPGEventManager::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject);
}

TStatId UARPGEventManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UARPGEventManager, STATGROUP_Tickables);
}

bool UARPGEventManager::RegisterARPGEventObject(UObject* ListenerObject, const FName& EventName, EARPGEventPriority Priority)
{
	if (!IsValid(ListenerObject))
	{
		UE_LOG(LogARPGEvent, Warning, TEXT("RegisterARPGEventObject: Invalid listener object for event '%s'"), *EventName.ToString());
		return false;
	}

	FName ListenerFuncName = GetARPGListenerFuncName(EventName);
	UFunction* ListenerFunc = ListenerObject->FindFunction(ListenerFuncName);
	if (!ListenerFunc)
	{
		UE_LOG(LogARPGEvent, Warning, TEXT("RegisterARPGEventObject: Function '%s' not found in object '%s'"),
			*ListenerFuncName.ToString(), *ListenerObject->GetName());
		return false;
	}

	TArray<FARPGObjectListener>& Listeners = EventListenerMap.FindOrAdd(EventName);

	for (const FARPGObjectListener& Listener : Listeners)
	{
		if (Listener.Object == ListenerObject)
		{
			return true;
		}
	}

	Listeners.Emplace(ListenerObject, ListenerFunc, Priority);
	Listeners.Sort();

	return true;
}

void UARPGEventManager::UnregisterARPGEventObject(UObject* ListenerObject, const FName& EventName)
{
	if (!IsValid(ListenerObject)) return;

	TArray<FARPGObjectListener>* Listeners = EventListenerMap.Find(EventName);
	if (!Listeners) return;

	Listeners->RemoveAll([ListenerObject](const FARPGObjectListener& Listener)
	{
		return Listener.Object == ListenerObject;
	});

	if (Listeners->IsEmpty())
	{
		EventListenerMap.Remove(EventName);
	}
}

void UARPGEventManager::UnregisterAllARPGEventsForActor(AActor* ListenerActor)
{
	if (!IsValid(ListenerActor)) return;

	for (auto It = EventListenerMap.CreateIterator(); It; ++It)
	{
		TArray<FARPGObjectListener>& Listeners = It.Value();
		Listeners.RemoveAll([ListenerActor](const FARPGObjectListener& Listener)
		{
			TObjectPtr<UObject> Obj = Listener.Object.Get();
			AActor* ObjAsActor = Cast<AActor>(Obj);
			return ObjAsActor == ListenerActor;
		});

		if (Listeners.IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}

void UARPGEventManager::UnregisterAllARPGEventsForObject(UObject* ListenerObject)
{
	if (!IsValid(ListenerObject)) return;

	for (auto It = EventListenerMap.CreateIterator(); It; ++It)
	{
		TArray<FARPGObjectListener>& Listeners = It.Value();
		Listeners.RemoveAll([ListenerObject](const FARPGObjectListener& Listener)
		{
			return Listener.Object == ListenerObject;
		});

		if (Listeners.IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}

void UARPGEventManager::DispatchARPGEvent(const FName& EventName, UARPGEventData* EventData)
{
	DispatchUObjectARPGEvent(EventName, EventData);
}

void UARPGEventManager::DispatchUObjectARPGEvent(const FName& EventName, UARPGEventData* EventData)
{
	if (!Config.bEnableQueue)
	{
		DispatchToListeners(EventName, EventData);
		return;
	}

	EventQueue.Emplace(EventName, EventData);

	if (IsValid(EventData))
	{
		QueueDataGuard.Add(EventData);
	}

	Stats.QueueMaxSize = FMath::Max(Stats.QueueMaxSize, EventQueue.Num());

	if (EventQueue.Num() >= Config.QueueWarningThreshold)
	{
		UE_LOG(LogARPGEvent, Warning, TEXT("Event queue size (%d) exceeds warning threshold (%d)"),
			EventQueue.Num(), Config.QueueWarningThreshold);
	}
}

void UARPGEventManager::DispatchARPGEventToTarget(AActor* TargetActor, const FName& EventName, UARPGEventData* EventData)
{
	if (!IsValid(TargetActor)) return;

	FName ListenerFuncName = GetARPGListenerFuncName(EventName);
	UFunction* TargetFunc = TargetActor->FindFunction(ListenerFuncName);
	if (TargetFunc)
	{
		TargetActor->ProcessEvent(TargetFunc, IsValid(EventData) ? &EventData : nullptr);
		Stats.TotalDispatched++;
	}
}

void UARPGEventManager::DispatchARPGEventToUObjectTarget(UObject* TargetObject, const FName& EventName, UARPGEventData* EventData)
{
	if (!IsValid(TargetObject)) return;

	FName ListenerFuncName = GetARPGListenerFuncName(EventName);
	UFunction* TargetFunc = TargetObject->FindFunction(ListenerFuncName);
	if (TargetFunc)
	{
		TargetObject->ProcessEvent(TargetFunc, IsValid(EventData) ? &EventData : nullptr);
		Stats.TotalDispatched++;
	}
}

void UARPGEventManager::FlushEventQueue()
{
	if (bIsProcessingQueue)
	{
		UE_LOG(LogARPGEvent, Warning, TEXT("FlushEventQueue: Already processing queue, skipping to prevent recursion"));
		return;
	}

	if (EventQueue.IsEmpty()) return;

	bIsProcessingQueue = true;

	int32 ProcessedCount = 0;
	int32 MaxToProcess = Config.MaxEventsPerFrame > 0 ? Config.MaxEventsPerFrame : EventQueue.Num();

	while (!EventQueue.IsEmpty() && ProcessedCount < MaxToProcess)
	{
		FARPGPendingEvent PendingEvent = EventQueue[0];
		EventQueue.RemoveAt(0);

		ConsumeEvent(PendingEvent);
		ProcessedCount++;
	}

	if (EventQueue.IsEmpty())
	{
		QueueDataGuard.Empty();
	}
	else
	{
		TSet<TObjectPtr<UARPGEventData>> ActiveData;
		for (const FARPGPendingEvent& Event : EventQueue)
		{
			if (IsValid(Event.EventData))
			{
				ActiveData.Add(Event.EventData);
			}
		}
		QueueDataGuard = ActiveData.Array();
	}

	bIsProcessingQueue = false;
}

void UARPGEventManager::ClearEventQueue()
{
	EventQueue.Empty();
	QueueDataGuard.Empty();
}

int32 UARPGEventManager::GetListenerCount(const FName& EventName) const
{
	const TArray<FARPGObjectListener>* Listeners = EventListenerMap.Find(EventName);
	return Listeners ? Listeners->Num() : 0;
}

void UARPGEventManager::ResetStats()
{
	Stats.TotalDispatched = 0;
	Stats.QueueMaxSize = 0;
	Stats.ListenersCount = 0;
}

void UARPGEventManager::ConsumeEvent(const FARPGPendingEvent& PendingEvent)
{
	DispatchToListeners(PendingEvent.EventName, PendingEvent.EventData);
}

void UARPGEventManager::DispatchToListeners(const FName& EventName, UARPGEventData* EventData)
{
	TArray<FARPGObjectListener>* Listeners = EventListenerMap.Find(EventName);
	if (!Listeners) return;

	int32 ValidListenerCount = 0;

	for (int32 i = Listeners->Num() - 1; i >= 0; i--)
	{
		FARPGObjectListener& Listener = (*Listeners)[i];

		UObject* Obj = Listener.Object.Get();
		if (!IsValid(Obj) || !Listener.Function)
		{
			Listeners->RemoveAtSwap(i);
			continue;
		}

		ValidListenerCount++;
		Obj->ProcessEvent(Listener.Function, IsValid(EventData) ? &EventData : nullptr);
	}

	Stats.TotalDispatched += ValidListenerCount;
	Stats.ListenersCount = ValidListenerCount;

	if (Listeners->IsEmpty())
	{
		EventListenerMap.Remove(EventName);
	}
}

void UARPGEventManager::CleanupInvalidListeners(const FName& EventName)
{
	TArray<FARPGObjectListener>* Listeners = EventListenerMap.Find(EventName);
	if (!Listeners) return;

	Listeners->RemoveAll([](const FARPGObjectListener& Listener)
	{
		return !Listener.Object.IsValid() || !Listener.Function;
	});

	if (Listeners->IsEmpty())
	{
		EventListenerMap.Remove(EventName);
	}
}

bool UARPGEventManager::IsValidARPGListenerFunction(UFunction* Func) const
{
	if (!Func) return false;
	if (Func->GetReturnProperty() != nullptr) return false;

	int32 ParamCount = 0;
	for (TFieldIterator<FProperty> It(Func); It; ++It)
	{
		if (It->HasAnyPropertyFlags(CPF_Parm) && !It->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			ParamCount++;
		}
	}

	return ParamCount <= 1;
}
