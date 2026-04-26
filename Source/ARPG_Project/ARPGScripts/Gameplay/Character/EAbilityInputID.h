#pragma once
#include "CoreMinimal.h"
#include "EAbilityInputID.generated.h"

UENUM(BlueprintType)
namespace EAbilityInputBinds
{
	enum EInputTypes : int
	{
		None			UMETA(DisplayName = "None"),
		Confirm			UMETA(DisplayName = "Confirm"),
		Cancel			UMETA(DisplayName = "Cancel"),
		Attack			UMETA(DisplayName = "Attack (LMB)"),
		Defense			UMETA(DisplayName = "Defense (RMB)"),
		DefaultSkill	UMETA(DisplayName = "Exclusive Skill"),
	};
}

