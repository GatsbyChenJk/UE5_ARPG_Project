#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FWidgetManifest.generated.h"

//UI 配置数据定义
USTRUCT(BlueprintType)
struct FUIWidgetConfig : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WidgetID;								// UI唯一标识（如"SelectRole"、"SelectMap"）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WidgetClass;			// UI蓝图路径
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WidgetOrder = 0;							// UI层级ZOrder（值越大，越在上层）
};