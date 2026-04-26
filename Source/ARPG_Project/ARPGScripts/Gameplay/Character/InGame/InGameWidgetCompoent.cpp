// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameWidgetCompoent.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "ARPGScripts/Widgets/Base/WidgetManager.h"

UInGameWidgetCompoent::UInGameWidgetCompoent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

UBaseWidget* UInGameWidgetCompoent::GetInventoryMenu()
{
	if (InventoryMenuWidget)
	{
		return InventoryMenuWidget;
	}
	else
	{
		return nullptr;
	}
}

UBaseWidget* UInGameWidgetCompoent::GetCharacterHUD()
{
	if (CharacterHUDWidget)
	{
		return CharacterHUDWidget;
	}
	else
	{
		return nullptr;
	}
}

UBaseWidget* UInGameWidgetCompoent::GetTargetLockWidget()
{
	return TargetLockWidget;
}

UBaseWidget* UInGameWidgetCompoent::GetInteractWidget()
{
	return InteractWidget;
}


void UInGameWidgetCompoent::BeginPlay()
{
	Super::BeginPlay();
	
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->OnWidgetDataTableLoaded.AddDynamic(this,&ThisClass::ConstructAllWidgets);
		GI->LoadWidgetDataTableAsync();
	}
	
}

// void UInGameWidgetCompoent::InitHUD()
// {
// 	if (GetOwner()->HasAuthority()) return;
// 	if (AAInGameCharacter* InGameChar = Cast<AAInGameCharacter>(OwningController.Get()->GetPawn()))
// 	{
// 		if (const auto AbilitySystemComponent = InGameChar->GetAbilitySystemComponent())
// 		{
// 			const UInGameCharacterAttributeSet* AttrSet = Cast<UInGameCharacterAttributeSet>(AbilitySystemComponent->GetAttributeSet(UInGameCharacterAttributeSet::StaticClass()));
//             	if (AttrSet)
//             	{
//             		// float CurrentHealth = AttrSet->GetHealth();
//             		// float MaxHealth = AttrSet->GetMaxHealth();
//             		// float AttackPower = AttrSet->GetAttackPower();
//             		// UE_LOG(LogTemp, Log, TEXT("Client attributes - Health: %.1f/%.1f, AttackPower: %.1f"), 
//             		// 	   CurrentHealth, MaxHealth, AttackPower);
//             
//             		UARPGEventData_OnCharacterAttributeChanged* EventData = NewObject<UARPGEventData_OnCharacterAttributeChanged>();
//             		EventData->InAttributeCount = AttrSet->GetHealth();
//             		EventData->AttributeBound = AttrSet->GetMaxHealth();
//             		ARPG_EVENT_SIMPLE(FName("OnHealthChanged"),EventData);
//             
//             		EventData->InAttributeCount = AttrSet->GetStamina();
//             		EventData->AttributeBound = AttrSet->GetMaxStamina();
//             		ARPG_EVENT_SIMPLE(FName("OnStaminaChanged"),EventData);
//             	}
// 		}
// 	}
// }

void UInGameWidgetCompoent::ConstructAllWidgets()
{
	Super::ConstructAllWidgets();
	
	InventoryMenuWidget = ConstructWidget(FString("1004"));
	CharacterHUDWidget = ConstructWidget(FString("1005"));
	TargetLockWidget = ConstructWidget(FString("1006"));
	//LoadingWidget = ConstructWidget(FString("1008"));
	InteractWidget = ConstructWidget(FString("1009"));
	
	CharacterHUDWidget->OnOpenWidget(OwningController.Get());
	InteractWidget->OpenWidget();
	//InitHUD();
}

