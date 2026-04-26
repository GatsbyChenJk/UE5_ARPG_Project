#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "ARPGEventManager.h"

#define GET_ARPG_EVENT_MANAGER() \
([&]() -> UARPGEventManager* { \
    if (GWorld) \
    { \
        if (UGameInstance* GI = GWorld->GetGameInstance()) \
        { \
            return GI->GetSubsystem<UARPGEventManager>(); \
        } \
    } \
    UE_LOG(LogARPGEvent, Warning, TEXT("GET_ARPG_EVENT_MANAGER failed: No GameInstance or EventManager")); \
    return nullptr; \
}())

#define ARPG_EVENT_ADD_UOBJECT(ListenerObj, EventName) \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->RegisterARPGEventObject(ListenerObj, EventName); } \
} while (0)

#define ARPG_EVENT_ADD_UOBJECT_WITH_PRIORITY(ListenerObj, EventName, Priority) \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->RegisterARPGEventObject(ListenerObj, EventName, Priority); } \
} while (0)

#define ARPG_EVENT_REMOVE_UOBJECT(ListenerObj, EventName) \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->UnregisterARPGEventObject(ListenerObj, EventName); } \
} while (0)

#define ARPG_EVENT_REMOVE_ALL(ListenerActor) \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->UnregisterAllARPGEventsForActor(ListenerActor); } \
} while (0)

#define ARPG_EVENT_REMOVE_ALL_OBJECT(ListenerObj) \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->UnregisterAllARPGEventsForObject(ListenerObj); } \
} while (0)

#define ARPG_EVENT_UOBJECT(EventName, EventDataObj) \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->DispatchUObjectARPGEvent(EventName, EventDataObj); } \
} while (0)

#define ARPG_EVENT_WITH_TARGET(TargetActor, EventName, EventDataObj) \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->DispatchARPGEventToTarget(TargetActor, EventName, EventDataObj); } \
} while (0)

#define ARPG_EVENT_WITH_UOBJECT_TARGET(TargetObject, EventName, EventDataObj) \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->DispatchARPGEventToUObjectTarget(TargetObject, EventName, EventDataObj); } \
} while (0)

#define ARPG_EVENT_FLUSH_QUEUE() \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->FlushEventQueue(); } \
} while (0)

#define ARPG_EVENT_CLEAR_QUEUE() \
do { \
    if (UARPGEventManager* EM = GET_ARPG_EVENT_MANAGER()) { EM->ClearEventQueue(); } \
} while (0)
