// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARPGScripts/Widgets/Composite/ARPGLeaf.h"
#include "ARPGLeaf_ShopItem.generated.h"

class UARPGLobbyPlayerStat;
class UButton;
class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class ARPG_PROJECT_API UARPGLeaf_ShopItem : public UARPGLeaf
{
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	
	virtual void NativeOnInitialized() override;
	
	UFUNCTION()
	void SetParentWidget(UARPGLobbyPlayerStat* InParent);
	
private:
	UPROPERTY(EditAnywhere,Category="ARPG UI")
	TObjectPtr<UTexture2D> Texture_ShopItemIcon;

	UPROPERTY(EditAnywhere,Category="ARPG UI")
	FString ShopItemID;

	UPROPERTY(EditAnywhere,Category="ARPG UI")
	FString ShopItemName;
	
	UPROPERTY()
	TObjectPtr<UARPGLobbyPlayerStat> ParentWidget;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_ShopItemName;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_ItemIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button_Select;

	UFUNCTION()
	void OnSelectShopItem();
};
