// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGLeaf_CharacterChoice.h"

#include "CharacterSelect.h"
#include "Components/Button.h"
#include "Components/Image.h"

void UARPGLeaf_CharacterChoice::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(Texture_CharacterIcon))
	{
		Image_CharacterIcon->SetBrushFromTexture(Texture_CharacterIcon);
	}
}

void UARPGLeaf_CharacterChoice::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_CharacterChoose->OnClicked.AddDynamic(this,&ThisClass::OnWidgetClicked);
}

void UARPGLeaf_CharacterChoice::SetParentWidget(UCharacterSelect* Parent)
{
	ParentWidget = Parent;
}

void UARPGLeaf_CharacterChoice::OnWidgetClicked()
{
	if (IsValid(ParentWidget))
	{
		ParentWidget->SetCurrentCharacterID(CharacterID);
		ParentWidget->OnCharacterChosen.Broadcast(CharacterID);
	}
}
