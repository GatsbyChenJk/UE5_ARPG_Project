#include "Player/InventoryPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "Interaction/Highlightable.h"
#include "Inventory/Component/InventoryComp.h"
#include "Items/Components/ItemComponent.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameWidgetCompoent.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "ARPGScripts/Gameplay/Base/GameModes/InGameMode.h"
#include "ARPGScripts/Gameplay/Base/SaveGameSystem/ARPGSaveGameManager.h"
#include "ARPGScripts/Gameplay/Combat/UInGameCharacterAttributeSet.h"
#include "ARPGScripts/Widgets/InGame/HUDWidgets/ARPGCharacterComposite.h"
#include "ARPGScripts/Widgets/InGame/HUDWidgets/ARPGLeaf_IconText.h"
#include "ARPGScripts/Widgets/InGame/HUDWidgets/ARPGLeaf_ProgressBar.h"
#include "ARPGScripts/Widgets/Base/BaseWidget.h"
#include "ARPGScripts/Widgets/Base/IInteractMessageWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/HUD/InventoryHUD.h"
#include "Widgets/Inventory/Spaitial/SpaitialInventoryComp.h"

AInventoryPlayerController::AInventoryPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	TraceDistance = 500.f;
	ItemTraceChannel = ECC_WorldStatic;
}

void AInventoryPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}

	InventoryComp = FindComponentByClass<UInventoryComp>();
	InGameWidgetComponent = FindComponentByClass<UInGameWidgetCompoent>();
	
	ARPG_EVENT_ADD_UOBJECT(this,FName("OnHealthChanged"));
	ARPG_EVENT_ADD_UOBJECT(this,FName("OnStaminaChanged"));
	ARPG_EVENT_ADD_UOBJECT(this,FName("OnMoneyChanged"));
	ARPG_EVENT_ADD_UOBJECT(this,FName("CharacterRespawn"));
}

void AInventoryPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TraceForItem();
}

void AInventoryPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(PrimaryInteraction,ETriggerEvent::Started,this,&AInventoryPlayerController::PrimaryInteract);
	EnhancedInputComponent->BindAction(ToggleInventoryAction,ETriggerEvent::Started,this,&AInventoryPlayerController::ToggleInventory);
	EnhancedInputComponent->BindAction(SaveGameAction,ETriggerEvent::Started,this,&AInventoryPlayerController::SaveGameInteract);
	//EnhancedInputComponent->BindAction(TestAction,ETriggerEvent::Started,this,&AInventoryPlayerController::TestSendARPGEvent);
}

void AInventoryPlayerController::ToggleInventory()
{
	if (!IsValid(InGameWidgetComponent) || !IsLocalController()) return;
	InGameWidgetComponent->ToggleWidget(FString("1004"));

	if (UBaseWidget* InteractWidget = InGameWidgetComponent->GetInteractWidget())
	{
		InteractWidget->ToggleWidget(this);
	}
}

UInGameWidgetCompoent* AInventoryPlayerController::GetInGameWidgetComponent()
{
	if (IsValid(InGameWidgetComponent))
	{
		return InGameWidgetComponent;
	}
	else
	{
		InGameWidgetComponent = FindComponentByClass<UInGameWidgetCompoent>();
		return InGameWidgetComponent;
	}
}

void AInventoryPlayerController::ShowInteractMessage()
{
	FString InteractMsg = InGameWidgetComponent->GetInteractMessage();
	ShowInteractMessage(InteractMsg);
}

void AInventoryPlayerController::ShowInteractMessage(const FString& Message)
{
	if (IsValid(InGameWidgetComponent))
	{
		if (UBaseWidget* InteractWidget = InGameWidgetComponent->GetInteractWidget())
		{
			if (InteractWidget->Implements<UInteractMessageWidget>())
			{
				IInteractMessageWidget::Execute_ShowPickUpMessage(InteractWidget, Message);
			}
		}
		InGameWidgetComponent->SetSceneInteractable(true);
	}
}

void AInventoryPlayerController::HideInteractMessage()
{
	if (IsValid(InGameWidgetComponent))
	{
		if (UBaseWidget* InteractWidget = InGameWidgetComponent->GetInteractWidget())
		{
			if (InteractWidget->Implements<UInteractMessageWidget>())
			{
				IInteractMessageWidget::Execute_HidePickUpMessage(InteractWidget);
			}
		}
		InGameWidgetComponent->SetSceneInteractable(false);
	}
}

void AInventoryPlayerController::OnCharacterDataSaved(bool bSuccess,int32 UserIndex)
{
	if (bSuccess)
	{
		Server_TravelToLobby();
	}
	//Server_TravelToLobby();
}

void AInventoryPlayerController::Server_TravelToLobby_Implementation()
{
	if (AInGameMode* GameMode = Cast<AInGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->Server_ReturnToLobby();
	}
}

void AInventoryPlayerController::ARPG_OnHealthChanged(UARPGEventData* EventData)
{
	UARPGEventData_OnCharacterAttributeChanged* Data = Cast<UARPGEventData_OnCharacterAttributeChanged>(EventData);
	if (IsValid(Data))
	{
		float Precent = Data->InAttributeCount / Data->AttributeBound;

		if (const auto CharacterHUD = Cast<UARPGCharacterComposite>(InGameWidgetComponent->GetCharacterHUD()))
		{
			if (const auto HealthBar = Cast<UARPGLeaf_ProgressBar>(CharacterHUD->FindChildByTag(HealthQueryTag)))
			{
				HealthBar->SetPercent(Precent);
			}
		}
	}
}

void AInventoryPlayerController::ARPG_OnStaminaChanged(UARPGEventData* EventData)
{
	UARPGEventData_OnCharacterAttributeChanged* Data = Cast<UARPGEventData_OnCharacterAttributeChanged>(EventData);
	if (IsValid(Data))
	{
		float Precent = Data->InAttributeCount / Data->AttributeBound;

		if (const auto CharacterHUD = Cast<UARPGCharacterComposite>(InGameWidgetComponent->GetCharacterHUD()))
		{
			if (const auto StaminaBar = Cast<UARPGLeaf_ProgressBar>(CharacterHUD->FindChildByTag(StaminaQueryTag)))
			{
				StaminaBar->SetPercent(Precent);
			}
		}
	}
}

void AInventoryPlayerController::ARPG_OnMoneyChanged(UARPGEventData* EventData)
{
	UARPGEventData_OnCharacterAttributeChanged* Data = Cast<UARPGEventData_OnCharacterAttributeChanged>(EventData);
	if (IsValid(Data))
	{
		float Money = Data->InAttributeCount;
		if (const auto CharacterHUD = Cast<UARPGCharacterComposite>(InGameWidgetComponent->GetCharacterHUD()))
		{
			if (const auto MoneyWidget = Cast<UARPGLeaf_IconText>(CharacterHUD->FindChildByTag(MoneyQueryTag)))
			{
				MoneyWidget->SetValueText(Money);
			}
		}
	}
}

void AInventoryPlayerController::ARPG_CharacterRespawn(UARPGEventData* EventData)
{
	if (IsValid(InGameWidgetComponent))
	{
		if (const auto InventoryWidget = Cast<USpaitialInventoryComp>(InGameWidgetComponent->GetInventoryMenu()))
		{
			InventoryWidget->CharacterEquipAllWeapons();
		}
	}
}


void AInventoryPlayerController::PrimaryInteract()
{
	// execute when trigger interact key
	// UE_LOG(LogTemp,Log,TEXT("Primary Interaction Triggered"))
	if (!InventoryComp.IsValid() || !CurActor.IsValid()) return;
	UItemComponent* IC = CurActor->FindComponentByClass<UItemComponent>();
	if (!IsValid(IC) || !InventoryComp.IsValid()) return;

	InventoryComp->TryAddItem(IC);
}

void AInventoryPlayerController::TraceForItem()
{
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport)) return;
	FVector2D ViewPortSize;
	GEngine->GameViewport->GetViewportSize(ViewPortSize);
	FVector2D ViewCenterLocation = ViewPortSize/2.0f;
	FVector TraceStart;
	FVector Forward;
	if (!UGameplayStatics::DeprojectScreenToWorld(this,ViewCenterLocation,TraceStart,Forward)) return;

	const FVector TraceEnd = TraceStart + (Forward * TraceDistance);
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult,TraceStart,TraceEnd,ItemTraceChannel);

	PrevActor = CurActor;
	CurActor = HitResult.GetActor();

	if (!CurActor.IsValid() && IsValid(InGameWidgetComponent))
	{
		if (UInventoryHUD* InteractWidget = Cast<UInventoryHUD>(InGameWidgetComponent->GetInteractWidget()))
		{
			if (InGameWidgetComponent->GetItemInteractable())
			{
				InteractWidget->HidePickUpMessage();
				InGameWidgetComponent->SetItemInteractable(false);
			}
		}

		// if (InGameWidgetComponent->GetItemInteractable())
		// {
		// 	HideInteractMessage();
		// }
	}

	if (PrevActor == CurActor) return;

	if (CurActor.IsValid())
	{
		//UE_LOG(LogInventory,Warning,TEXT("Perform Logic when trace hit Cur actor"))
		if (UActorComponent* itemHighlightComp = CurActor->FindComponentByInterface(UHighlightable::StaticClass());IsValid(itemHighlightComp))
		{
			IHighlightable::Execute_Highlight(itemHighlightComp);
		}
		
		if (const auto itemComp = CurActor->FindComponentByClass<UItemComponent>())
		{
			if (IsValid(InGameWidgetComponent))
			{
				if (UInventoryHUD* InteractWidget = Cast<UInventoryHUD>(InGameWidgetComponent->GetInteractWidget()))
				{
					InteractWidget->ShowPickUpMessage(itemComp->GetPickUpMessage());
				}
				InGameWidgetComponent->SetItemInteractable(true);
				//ShowInteractMessage(itemComp->GetPickUpMessage());
			}
		}
	}
	
	if (PrevActor.IsValid())
	{
		//UE_LOG(LogInventory,Warning,TEXT("Perform Logic when trace hit prev actor"))
		if (UActorComponent* itemHighlightComp = PrevActor->FindComponentByInterface(UHighlightable::StaticClass());IsValid(itemHighlightComp))
		{
			IHighlightable::Execute_UnHighlight(itemHighlightComp);
		}
	}
}

void AInventoryPlayerController::SaveGameInteract()
{
	if (InGameWidgetComponent->GetSceneInteractable())
	{
		IARPGSaveGameFunc::Execute_SaveCharacterData(this,Cast<AARPGBaseCharacter>(GetPawn()));
	}
}

void AInventoryPlayerController::SaveCharacterData_Implementation(AARPGBaseCharacter* InPlayer)
{
	if (!IsValid(InPlayer)) return;

	const auto PlayerAttribute = InPlayer->GetAttributeSet();
	if (!IsValid(PlayerAttribute)) return;

	if (const auto SaveGameMgr = GetGameInstance()->GetSubsystem<UARPGSaveGameManager>())
	{
		SaveGameMgr->SynchronousLoadGame(0);
		FARPGPlayerData PlayerData = SaveGameMgr->GetPlayerData();
		PlayerData.ResetMod();

		PlayerData.PlayerTotalMoney += PlayerAttribute->GetCharacterMoney();

		SaveGameMgr->OnSaveCompleted.AddDynamic(this,&ThisClass::OnCharacterDataSaved);
		SaveGameMgr->AsyncSaveGame(PlayerData,0);
	}
}


