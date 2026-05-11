// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ARPG_ProjectCharacter.h"
#include "GenericTeamAgentInterface.h"
#include "ARPGBaseCharacter.generated.h"

class UARPGAbilitySystemComponent;
class UARPGMontageNetTransport;

class UCharacterHealthManager;
class UStaminaManagerComponent;
class UCharacterShieldComponent;
class URollComponent;
class USprintComponent;
class UCharacterAttackComponent;
class UCharacterDamageComponent;
class UInGameCharacterAttributeSet;

/**
 * 通用角色基类 - 玩家/AI的共同父类
 * 核心：只封装通用组件+通用接口，不持有GAS对象，GAS对象由子类实现获取逻辑
 * 玩家：从PlayerState拿GAS对象 | AI：自行创建GAS对象
 */

UCLASS(Abstract)
class ARPG_PROJECT_API AARPGBaseCharacter : public AARPG_ProjectCharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AARPGBaseCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// GAS start
	/** 返回原生 ASC 基类指针（兼容 IAbilitySystemInterface / 现有调用方） */
	UFUNCTION(BlueprintCallable, Category = "Character|GAS")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const { return nullptr; };

	/** 返回类型安全的 ARPG 自定义 ASC；需要 Socket 复制/多 Mesh 蒙太奇时使用此接口 */
	UFUNCTION(BlueprintCallable, Category = "Character|GAS")
	virtual UARPGAbilitySystemComponent* GetARPGAbilitySystemComponent() const { return nullptr; };

	UFUNCTION(BlueprintCallable, Category = "Character|GAS")
	virtual const UInGameCharacterAttributeSet* GetAttributeSet() const { return nullptr; };
	// GAS end
	
	// Character Combat start
	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	virtual UCharacterHealthManager* GetHealthManagerComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	virtual UCharacterDamageComponent* GetCharacterDamageComponent();

	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	virtual UCharacterAttackComponent* GetCharacterAttackComponent();

	UFUNCTION(BlueprintCallable, Category = "Character|Combat")
	virtual void CharacterDeath();

	// bool GetIsDead(){return bIsDying;};
	//
	// void SetIsDead(bool bValue){bIsDying=bValue;};
	// Character Combat end

	// Character Special Operation start
	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	virtual UStaminaManagerComponent* GetStaminaManagerComponent() const;
	
	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	virtual USprintComponent* GetSprintComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Sprint")
	virtual void SetSprintConsumeHandle(FActiveGameplayEffectHandle InHandle);

	UFUNCTION(BlueprintCallable, Category = "Character|Sprint")
	virtual void SetSprintRecoverHandle(FActiveGameplayEffectHandle InHandle);

	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	virtual URollComponent* GetRollComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Character|Components")
	virtual UCharacterShieldComponent* GetCharacterShieldComponent() const;
	// Character Special Operation end

	// Team Affiliation start
public:
	/** 获取角色阵营ID */
	UFUNCTION(BlueprintCallable, Category = "Character|Team")
	virtual int32 GetTeamID() const { return DefaultTeamID; }

	/** 设置角色阵营（服务器调用） */
	UFUNCTION(BlueprintCallable, Category = "Character|Team")
	virtual void SetTeamID(int32 InTeamID);

	/** IGenericTeamAgentInterface - 获取团队ID */
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(GetTeamID()); }

	/** IGenericTeamAgentInterface - 设置团队ID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override { SetTeamID(NewTeamID.GetId()); }

	/** 检查目标是否为敌对关系 */
	UFUNCTION(BlueprintCallable, Category = "Character|Team")
	virtual bool IsHostileTo(AActor* OtherActor) const;

	/** 检查目标是否为友方 */
	UFUNCTION(BlueprintCallable, Category = "Character|Team")
	virtual bool IsFriendlyTo(AActor* OtherActor) const;

protected:
	/** 默认阵营ID（可在子类或蓝图中覆盖） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	int32 DefaultTeamID = 0;
	// Team Affiliation end

protected:

	/** Network transport for UARPGMeshSyncMontageProxy. Holds the Server RPC
	 *  and replicated property so the proxy can sync mesh transforms without
	 *  adding replication logic to this class. */
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Animation")
	// TObjectPtr<UARPGMontageNetTransport> MontageNetTransport;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCharacterDamageComponent* CachedDamageComp;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCharacterAttackComponent* CachedAttackComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USprintComponent* CachedSprintComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	URollComponent* CachedRollComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCharacterShieldComponent* CachedShieldComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaminaManagerComponent* CachedStaminaMgrComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCharacterHealthManager* CachedHealthMgrComp;

	// combat state
	// UPROPERTY(Replicated)
	// bool bIsDying;
};
