#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FCharacterManifest.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UImage;

USTRUCT(BlueprintType)
struct FCharacterAttributeModifier
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText AttributeName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttributeValue;
};

USTRUCT(BlueprintType)
struct FCharacterManifest : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CharacterID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<APawn> CharacterClass;

	// Lobby CharacterData start
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCharacterAttributeModifier> CharacterAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> CharacterPreviewAnimAsset;
	// Lobby CharacterData end

	// InGame Character Data start
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> CharacterInitData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> CharacterBuffEffect;

	// TODO : 改为角色专属技能
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> CharacterMainAbility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> CharacterHurtAbility;
	//  InGame Character Data end
};

USTRUCT(BlueprintType)
struct FCharacterData 
{
	GENERATED_BODY()

	FCharacterData() {}

	void SetCharacterManifest(FCharacterManifest* NewManifest) { CharacterManifest = NewManifest; };

	FCharacterManifest* GetCharacterManifest() const {return CharacterManifest;};

	bool bIsCharacterSet{false};

private:
	FCharacterManifest* CharacterManifest{};
};

