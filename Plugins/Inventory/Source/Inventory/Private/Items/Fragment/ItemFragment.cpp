#include "Items/Fragment/ItemFragment.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "ARPGScripts/Gameplay/Base/ARPGCharacter/ARPGBaseCharacter.h"
#include "ARPGScripts/Gameplay/Base/ARPGEventHandler/ARPGEventMacros.h"
#include "Equipment/EquipActor/InventoryEquipActor.h"
#include "Widgets/Composite/InventoryCompositeBase.h"
#include "Widgets/Composite/InventoryLeafImage.h"
#include "Widgets/Composite/InventoryLeafLabelValue.h"
#include "Widgets/Composite/InventoryLeafText.h"
#include "GameplayEffect.h"
#include "ARPGScripts/Gameplay/Character/InGame/AInGameCharacter.h"
#include "ARPGScripts/Gameplay/Character/InGame/InGameAICharacter.h"

void FInventoryItemFragment::Assimilate(UInventoryCompositeBase* Composite) const
{
	if (!MatchesWidgetTag(Composite)) return;
	Composite->Expand();
}

bool FInventoryItemFragment::MatchesWidgetTag(const UInventoryCompositeBase* Composite) const
{
	return Composite->GetFragmentTag().MatchesTagExact(GetFragmentTag());
}

void FImageFragment::Assimilate(UInventoryCompositeBase* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite);

	if (!MatchesWidgetTag(Composite)) return;

	UInventoryLeafImage* Image = Cast<UInventoryLeafImage>(Composite);
	if (!IsValid(Image)) return;

	Image->SetImage(Icon);
	Image->SetBoxSize(IconDimensions);
	Image->SetImageSize(IconDimensions);
}

void FTextFragment::Assimilate(UInventoryCompositeBase* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;

	UInventoryLeafText* LeafText = Cast<UInventoryLeafText>(Composite);
	if (!IsValid(LeafText)) return;

	LeafText->SetText(FragmentText);
}

void FLabeledNumberFragment::Assimilate(UInventoryCompositeBase* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite);

	if (!MatchesWidgetTag(Composite)) return;
	
	UInventoryLeafLabelValue* LabeledValue = Cast<UInventoryLeafLabelValue>(Composite);
	if (!IsValid(LabeledValue)) return;

	LabeledValue->SetText_Label(Text_Label, bCollapseLabel);
	
	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = MinFractionalDigits;
	Options.MaximumFractionalDigits = MaxFractionalDigits;
	
	LabeledValue->SetText_Value(FText::AsNumber(Value, &Options), bCollapseValue);
}

void FLabeledNumberFragment::Manifest()
{
	FInventoryItemFragment::Manifest();

	if (bRandomizeOnManifest)
	{
		Value = FMath::FRandRange(Min, Max);
	}
	bRandomizeOnManifest = false;
}

void FSellableFragment::OnSell(APlayerController* PC)
{
	ApplyGameplayEffectByValue(PC,GetValue());
}

void FSellableFragment::ApplyGameplayEffectByValue(APlayerController* PC, float InValue)
{
	if (!IsValid(PC)) return;
	// get character asc
	if (AARPGBaseCharacter* OwnerChar = Cast<AARPGBaseCharacter>(PC->GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = OwnerChar->GetAbilitySystemComponent())
		{
			// make GE
			auto EffectCtx = ASC->MakeEffectContext();
			EffectCtx.AddSourceObject(ASC);
			
			// apply to owner 
			if (!IsValid(ModifyEffectClass)) return;
			FGameplayEffectSpecHandle ModifySpec = ASC->MakeOutgoingSpec(ModifyEffectClass,1,EffectCtx);
			if (ModifySpec.IsValid())
			{
				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(ModifySpec,ModifyTag,InValue);
				ASC->ApplyGameplayEffectSpecToSelf(*ModifySpec.Data.Get());
			}
			
		}
	}
}

void FConsumeModifier::ApplyGameplayEffectByValue(APlayerController* PC, float InValue)
{
	if (!IsValid(PC)) return;
	// get character asc
	if (AARPGBaseCharacter* OwnerChar = Cast<AARPGBaseCharacter>(PC->GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = OwnerChar->GetAbilitySystemComponent())
		{
			// make GE
			auto EffectCtx = ASC->MakeEffectContext();
			EffectCtx.AddSourceObject(ASC);
			
			// apply to owner 
			if (!IsValid(ModifyEffectClass)) return;
			FGameplayEffectSpecHandle ModifySpec = ASC->MakeOutgoingSpec(ModifyEffectClass,1,EffectCtx);
			if (ModifySpec.IsValid())
			{
				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(ModifySpec,ModifyTag,InValue);
				ASC->ApplyGameplayEffectSpecToSelf(*ModifySpec.Data.Get());
			}
			
		}
	}
}

void FConsumableFragment::OnConsume(APlayerController* PlayerController)
{
	for (auto& Modifier : ConsumeModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.OnConsume(PlayerController);
	}
}

void FConsumableFragment::Assimilate(UInventoryCompositeBase* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite);
	for (const auto& Modifier : ConsumeModifiers)
	{
		const auto& ModRef = Modifier.Get();
		ModRef.Assimilate(Composite);
	}
}

void FConsumableFragment::Manifest()
{
	FInventoryItemFragment::Manifest();
	for (auto& Modifier : ConsumeModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.Manifest();
	}
}

void FHealthFragment::OnConsume(APlayerController* PlayerController)
{
	// GEngine->AddOnScreenDebugMessage(-1,
	// 	5.0f,
	// 	FColor::Red,
	// 	FString::Printf(TEXT("Healing Value:%f"),
	// 		GetValue()));
	ApplyGameplayEffectByValue(PlayerController,GetValue());
}

void FManaFragment::OnConsume(APlayerController* PlayerController)
{
	// GEngine->AddOnScreenDebugMessage(-1,
	// 	5.0f,
	// 	FColor::Red,
	// 	FString::Printf(TEXT("Mana Value:%f"),
	// 		GetValue()));
	ApplyGameplayEffectByValue(PlayerController,GetValue());
}

void FEquipmentModifier::ApplyGameplayEffectByValue(APlayerController* PC, float InValue)
{
	if (!IsValid(PC)) return;
	// get character asc
	if (AARPGBaseCharacter* OwnerChar = Cast<AARPGBaseCharacter>(PC->GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = OwnerChar->GetAbilitySystemComponent())
		{
			// make GE
			auto EffectCtx = ASC->MakeEffectContext();
			EffectCtx.AddSourceObject(ASC);
			
			// apply to owner 
			if (!IsValid(ModifyEffectClass)) return;
			FGameplayEffectSpecHandle ModifySpec = ASC->MakeOutgoingSpec(ModifyEffectClass,1,EffectCtx);
			if (ModifySpec.IsValid())
			{
				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(ModifySpec,ModifyTag,InValue);
				ASC->ApplyGameplayEffectSpecToSelf(*ModifySpec.Data.Get());
			}
			
		}
	}
}

void FEquipmentModifier::ApplyGameplayEffectByValue(AAIController* AIC, float InValue)
{
	if (!IsValid(AIC)) return;
	// get character asc
	if (AARPGBaseCharacter* OwnerChar = Cast<AARPGBaseCharacter>(AIC->GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = OwnerChar->GetAbilitySystemComponent())
		{
			// make GE
			auto EffectCtx = ASC->MakeEffectContext();
			EffectCtx.AddSourceObject(ASC);
			
			// apply to owner 
			if (!IsValid(ModifyEffectClass)) return;
			FGameplayEffectSpecHandle ModifySpec = ASC->MakeOutgoingSpec(ModifyEffectClass,1,EffectCtx);
			if (ModifySpec.IsValid())
			{
				UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(ModifySpec,ModifyTag,InValue);
				ASC->ApplyGameplayEffectSpecToSelf(*ModifySpec.Data.Get());
			}
			
		}
	}
}

void FStrengthModifier::OnEquip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f,
		FColor::Green,
		FString::Printf(TEXT("Strength increased by: %f"),
			GetValue()));
}

void FStrengthModifier::OnUnequip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f,
		FColor::Green,
		FString::Printf(TEXT("Strength decreased by: %f"),
			GetValue()));
}

void FArmorModifier::OnEquip(APlayerController* PC)
{
	// GEngine->AddOnScreenDebugMessage(
	// 	-1,
	// 	5.f,
	// 	FColor::Green,
	// 	FString::Printf(TEXT("Armor increased by: %f"),
	// 		GetValue()));

	ApplyGameplayEffectByValue(PC,GetValue());
}

void FArmorModifier::OnUnequip(APlayerController* PC)
{
	// GEngine->AddOnScreenDebugMessage(
	// 	-1,
	// 	5.f,
	// 	FColor::Green,
	// 	FString::Printf(TEXT("Armor decreased by: %f"),
	// 		GetValue()));

	ApplyGameplayEffectByValue(PC,-1.0f * GetValue());
}

void FDamageModifier::OnEquip(APlayerController* PC)
{
	// GEngine->AddOnScreenDebugMessage(
	// 	-1,
	// 	5.f,
	// 	FColor::Green,
	// 	FString::Printf(TEXT("Damage increased by: %f"),
	// 		GetValue()));

	ApplyGameplayEffectByValue(PC,GetValue());
}

void FDamageModifier::OnUnequip(APlayerController* PC)
{
	// GEngine->AddOnScreenDebugMessage(
	// 	-1,
	// 	5.f,
	// 	FColor::Green,
	// 	FString::Printf(TEXT("Damage decreased by: %f"),
	// 		GetValue()));
	
	ApplyGameplayEffectByValue(PC,-1.0f * GetValue());
}

void FDamageModifier::OnAIEquip(AAIController* AIC)
{
	ApplyGameplayEffectByValue(AIC,GetValue());
}

void FDamageModifier::OnAIUnequip(AAIController* AIC)
{
	ApplyGameplayEffectByValue(AIC,-1.0f * GetValue());
}

void FStaminaCostModifier::OnEquip(APlayerController* PC)
{
	ApplyGameplayEffectByValue(PC,GetValue());
}

void FStaminaCostModifier::OnUnequip(APlayerController* PC)
{
	ApplyGameplayEffectByValue(PC,-1.0f * GetValue());
}

void FStaminaCostModifier::OnAIEquip(AAIController* AIC)
{
	ApplyGameplayEffectByValue(AIC,GetValue());
}

void FStaminaCostModifier::OnAIUnequip(AAIController* AIC)
{
	ApplyGameplayEffectByValue(AIC,-1.0f * GetValue());
}

void FEquipmentFragment::OnEquip(APlayerController* PC)
{
	if (bEquipped) return;
	bEquipped = true;
	
	UARPGEventData_Weapon* EventData = NewObject<UARPGEventData_Weapon>();
	EventData->EquipWeaponType = GetWeaponConfigType();
	EventData->WeaponID = GetConfigID();
	EventData->WeaponActor = EquippedActor.Get();

	if (auto Char = Cast<AAInGameCharacter>(PC->GetPawn()))
	{
		ARPG_EVENT_WITH_UOBJECT_TARGET(Char,FName("CharacterEquipWeapon"),EventData);	
	}
	
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.OnEquip(PC);
	}
}

void FEquipmentFragment::OnUnequip(APlayerController* PC)
{
	if (!bEquipped) return;
	bEquipped = false;

	UARPGEventData_Weapon* EventData = NewObject<UARPGEventData_Weapon>();
	EventData->EquipWeaponType = GetWeaponConfigType();
	// ARPG_EVENT_UOBJECT(FName("CharacterUnequipWeapon"),EventData);

	if (auto Char = Cast<AAInGameCharacter>(PC->GetPawn()))
	{
		ARPG_EVENT_WITH_UOBJECT_TARGET(Char,FName("CharacterUnequipWeapon"),EventData);	
	}
	
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.OnUnequip(PC);
	}
}

void FEquipmentFragment::OnAIEquip(AAIController* AIC)
{
	if (bEquipped) return;
	bEquipped = true;
	
	UARPGEventData_Weapon* EventData = NewObject<UARPGEventData_Weapon>();
	EventData->EquipWeaponType = GetWeaponConfigType();
	EventData->WeaponID = GetConfigID();
	EventData->WeaponActor = EquippedActor.Get();

	if (auto AIChar = Cast<AInGameAICharacter>(AIC->GetPawn()))
	{
		ARPG_EVENT_WITH_UOBJECT_TARGET(AIChar,FName("AIEquipWeapon"),EventData);
	}

	//ARPG_EVENT_UOBJECT(FName("AIEquipWeapon"),EventData);
	
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.OnAIEquip(AIC);
	}
}

void FEquipmentFragment::OnAIUnequip(AAIController* AIC)
{
	if (!bEquipped) return;
	bEquipped = false;
	
	UARPGEventData_Weapon* EventData = NewObject<UARPGEventData_Weapon>();
	EventData->EquipWeaponType = GetWeaponConfigType();

	if (auto AIChar = Cast<AInGameAICharacter>(AIC->GetPawn()))
	{
		ARPG_EVENT_WITH_UOBJECT_TARGET(AIChar,FName("AIUnequipWeapon"),EventData);
	}
	
	//ARPG_EVENT_UOBJECT(FName("AIUnequipWeapon"),EventData);
	
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.OnAIUnequip(AIC);
	}
}

void FEquipmentFragment::Assimilate(UInventoryCompositeBase* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite);
	for (const auto& Modifier : EquipModifiers)
	{
		const auto& ModRef = Modifier.Get();
		ModRef.Assimilate(Composite);
	}
}

void FEquipmentFragment::Manifest()
{
	FInventoryItemFragment::Manifest();
	for (auto& Modifier : EquipModifiers)
	{
		auto& ModRef = Modifier.GetMutable();
		ModRef.Manifest();
	}
}

AInventoryEquipActor* FEquipmentFragment::SpawnAttachedActor(USkeletalMeshComponent* AttachMesh) const
{
	if (!IsValid(EquipActorClass) || !IsValid(AttachMesh)) return nullptr;

	// TODO: Change Actor class type to use in attackcomponent
	AInventoryEquipActor* SpawnedActor = AttachMesh->GetWorld()->SpawnActor<AInventoryEquipActor>(EquipActorClass);
	SpawnedActor->AttachToComponent(AttachMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketAttachPoint);

	return SpawnedActor;
}

void FEquipmentFragment::DestroyAttachedActor() const
{
	if (EquippedActor.IsValid())
	{
		EquippedActor->Destroy();
	}
}

void FEquipmentFragment::SetEquippedActor(AInventoryEquipActor* EquipActor)
{
	EquippedActor = EquipActor;
}



