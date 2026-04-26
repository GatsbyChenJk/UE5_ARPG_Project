#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Base/BaseWidget.h"
#include "LoadingScreenWidget.generated.h"

class UTexture2D;

UCLASS()
class ARPG_PROJECT_API ULoadingScreenWidget : public UBaseWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "LoadingScreen")
	void OnMapInfoSet(UTexture2D* MapIcon, const FString& MapName);

	virtual void OnOpenWidget(APlayerController* OwningController) override;
	virtual void OnCloseWidget(APlayerController* OwningController) override;
};
