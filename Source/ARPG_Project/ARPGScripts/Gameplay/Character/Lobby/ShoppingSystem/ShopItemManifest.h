#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShopItemManifest.generated.h"

USTRUCT(BlueprintType)
struct FShopItemManifest : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ItemPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPowerUp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefensePowerUp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthPowerUp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemDesc;
	
};
