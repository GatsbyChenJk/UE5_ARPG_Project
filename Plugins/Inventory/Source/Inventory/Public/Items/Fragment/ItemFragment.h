#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ARPGScripts/Gameplay/Weapon/FWeaponManifest.h"
#include "StructUtils/InstancedStruct.h"

#include "ItemFragment.generated.h"

class UGameplayEffect;
enum EWeaponType : uint8;
class AAIController;
class AInventoryEquipActor;
class APlayerController;

USTRUCT(BlueprintType)
struct FInventoryFragment
{
	GENERATED_BODY()
	FInventoryFragment() {}
	FInventoryFragment(const FInventoryFragment&) = default;
	FInventoryFragment& operator=(const FInventoryFragment&) = default;
	FInventoryFragment(FInventoryFragment&&) = default;
	FInventoryFragment& operator=(FInventoryFragment&&) = default;
	virtual ~FInventoryFragment() {}

	FGameplayTag GetFragmentTag() const {return FragmentTag;}
	void SetFragmentTag(FGameplayTag Tag) { FragmentTag = Tag; }
	FString GetConfigID() {return ItemConfigID;}
	virtual void Manifest() {}
private:
	UPROPERTY(EditAnywhere,Category="Inventory")
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere,Category="Inventory")
	FString ItemConfigID;
};

/*
 * Item fragment specifically for assimilation into a widget.
 */
class UInventoryCompositeBase;
USTRUCT(BlueprintType)
struct FInventoryItemFragment : public FInventoryFragment
{
	GENERATED_BODY()

	virtual void Assimilate(UInventoryCompositeBase* Composite) const;
protected:
	bool MatchesWidgetTag(const UInventoryCompositeBase* Composite) const;
};

USTRUCT(blueprintType)
struct FGridFragment : public FInventoryFragment
{
	GENERATED_BODY()

	FIntPoint GetGridSize() const {return GridSize;}
	void SetGridSize(const FIntPoint& Size) {this->GridSize = Size;}
	float GetGridPadding() const {return GridPadding;}
	void SetGridPadding(float Padding) {this->GridPadding = Padding;}

private:
	UPROPERTY(EditAnywhere,Category="Inventory")
	FIntPoint GridSize{1,1};

	UPROPERTY(EditAnywhere,Category="Inventory")
	float GridPadding{0.f};
};

USTRUCT(BlueprintType)
struct FImageFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

	UTexture2D* GetIcon() const {return Icon;}
	virtual void Assimilate(UInventoryCompositeBase* Composite) const;
private:
	UPROPERTY(EditAnywhere,Category="Inventory")
	TObjectPtr<UTexture2D> Icon = nullptr;
	UPROPERTY(EditAnywhere,Category="Inventory")
	FVector2D IconDimensions{44.f,44.f};
};

USTRUCT(BlueprintType)
struct FTextFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

	FText GetText() const { return FragmentText; }
	void SetText(const FText& Text) { FragmentText = Text; }
	virtual void Assimilate(UInventoryCompositeBase* Composite) const override;

private:

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText FragmentText;
};

USTRUCT(BlueprintType)
struct FLabeledNumberFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

	virtual void Assimilate(UInventoryCompositeBase* Composite) const override;
	virtual void Manifest() override;
	float GetValue() const { return Value; }
	
	virtual void ApplyGameplayEffectByValue(APlayerController* PC,float InValue){};
	virtual void ApplyGameplayEffectByValue(AAIController* AIC,float InValue){};
	// When manifesting for the first time, this fragment will randomize. However, onee equipped
	// and dropped, an item should retain the same value, so randomization should not occur.
	bool bRandomizeOnManifest{true};

private:

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText Text_Label{};

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	float Value{0.f};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float Min{0};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float Max{0};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bCollapseLabel{false};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bCollapseValue{false};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MinFractionalDigits{1};
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxFractionalDigits{1};
protected:
	UPROPERTY(EditAnywhere, Category = "Inventory | GAS")
	TSubclassOf<UGameplayEffect> ModifyEffectClass;

	UPROPERTY(EditAnywhere, Category = "Inventory | GAS")
	FGameplayTag ModifyTag;
};

USTRUCT(BlueprintType)
struct FStackableFragment : public FInventoryFragment
{
	GENERATED_BODY()

	int32 GetMaxStackSize() const {return MaxStackSize;}
	int32 GetStackCount() const {return StackCount;}

	void SetStackCount(int32 Count) {StackCount = Count;}
	
private:
	UPROPERTY(EditAnywhere,Category="Inventory")
	int32 MaxStackSize{1};

	UPROPERTY(EditAnywhere,Category="Inventory")
	int32 StackCount{1};
};

// Sell Fragment Start

USTRUCT(BlueprintType)
struct FSellableFragment : public FLabeledNumberFragment
{
	GENERATED_BODY()

	virtual void OnSell(APlayerController* PC);
	virtual void ApplyGameplayEffectByValue(APlayerController* PC, float InValue) override;
};

// Sell Fragment End


// Consume Fragments Start

USTRUCT(BlueprintType)
struct FConsumeModifier : public FLabeledNumberFragment
{
	GENERATED_BODY()

	virtual void OnConsume(APlayerController* PC) {}
	virtual void ApplyGameplayEffectByValue(APlayerController* PC, float InValue) override;
};

USTRUCT(BlueprintType)
struct FConsumableFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

	virtual void OnConsume(APlayerController* PlayerController);
	virtual void Assimilate(UInventoryCompositeBase* Composite) const override;
	virtual void Manifest() override;
private:

	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FConsumeModifier>> ConsumeModifiers;
};

USTRUCT(BlueprintType)
struct FHealthFragment : public FConsumeModifier
{
	GENERATED_BODY()

	virtual void OnConsume(APlayerController* PlayerController) override;
};

USTRUCT(BlueprintType)
struct FManaFragment : public FConsumeModifier
{
	GENERATED_BODY()
	
	virtual void OnConsume(APlayerController* PlayerController) override;
};

// Consume Fragments End

// Equip Fragments Start
USTRUCT(BlueprintType)
struct FEquipmentModifier : public FLabeledNumberFragment
{
	GENERATED_BODY()

	virtual void OnEquip(APlayerController* PC) {}
	virtual void OnUnequip(APlayerController* PC) {}
	virtual void OnAIEquip(AAIController* AIC) {}
	virtual void OnAIUnequip(AAIController* AIC) {}

	virtual void ApplyGameplayEffectByValue(APlayerController* PC, float InValue) override;
	virtual void ApplyGameplayEffectByValue(AAIController* AIC, float InValue) override;
};

USTRUCT(BlueprintType)
struct FStrengthModifier : public FEquipmentModifier
{
	GENERATED_BODY()

	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnequip(APlayerController* PC) override;
};

USTRUCT(BlueprintType)
struct FArmorModifier : public FEquipmentModifier
{
	GENERATED_BODY()

	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnequip(APlayerController* PC) override;
};

USTRUCT(BlueprintType)
struct FDamageModifier : public FEquipmentModifier
{
	GENERATED_BODY()

	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnequip(APlayerController* PC) override;

	virtual void OnAIEquip(AAIController* AIC) override;
	virtual void OnAIUnequip(AAIController* AIC) override;
};

USTRUCT(BlueprintType)
struct FStaminaCostModifier : public FEquipmentModifier
{
	GENERATED_BODY()

	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnequip(APlayerController* PC) override;

	virtual void OnAIEquip(AAIController* AIC) override;
	virtual void OnAIUnequip(AAIController* AIC) override;
};

USTRUCT(BlueprintType)
struct FEquipmentFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

	bool bEquipped{false};
	void OnEquip(APlayerController* PC);
	void OnUnequip(APlayerController* PC);

	void OnAIEquip(AAIController* AIC);
	void OnAIUnequip(AAIController* AIC);
	
	virtual void Assimilate(UInventoryCompositeBase* Composite) const override;
	virtual void Manifest() override;

	AInventoryEquipActor* SpawnAttachedActor(USkeletalMeshComponent* AttachMesh) const;
	void DestroyAttachedActor() const;
	FGameplayTag GetEquipmentType() const { return EquipmentType; }
	void SetEquippedActor(AInventoryEquipActor* EquipActor);
	EWeaponType GetWeaponConfigType() const {return WeaponConfigType;}
	TSubclassOf<AInventoryEquipActor> GetEquipActorClass() const { return EquipActorClass; }
	FName GetSocketAttachPoint() const { return SocketAttachPoint; }
	
private:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FEquipmentModifier>> EquipModifiers;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<AInventoryEquipActor> EquipActorClass = nullptr;

	TWeakObjectPtr<AInventoryEquipActor> EquippedActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FName SocketAttachPoint{NAME_None};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere,Category= "Inventory")
	TEnumAsByte<EWeaponType> WeaponConfigType = EWeaponType::Unarmed;
};

// Equip Fragments End
