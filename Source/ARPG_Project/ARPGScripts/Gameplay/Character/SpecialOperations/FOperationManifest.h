#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FOperationManifest.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FOperationManifest : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> OperationAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OperationStaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaRecoverWindow;
};