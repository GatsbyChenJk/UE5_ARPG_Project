// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGLeaf_CharaterText.h"

#include "CharacterSelect.h"
#include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
#include "Components/TextBlock.h"

void UARPGLeaf_CharacterText::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (IsValid(Text_AttributeName))
	{
		SetText_Name(AttributeName,false);
	}
}

void UARPGLeaf_CharacterText::SetParentWidget(UCharacterSelect* Parent)
{
	ParentWidget = Parent;
	if (IsValid(ParentWidget))
	{
		ParentWidget->OnCharacterChosen.AddDynamic(this,&ThisClass::OnCharacterChosen);
	}
}

void UARPGLeaf_CharacterText::SetText_Name(const FText& Text, bool bCollapse)
{
	if (bCollapse)
	{
		Text_AttributeName->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	Text_AttributeName->SetText(Text);
}

void UARPGLeaf_CharacterText::SetText_Value(const FText& Text, bool bCollapse)
{
	if (bCollapse)
	{
		Text_AttributeValue->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	Text_AttributeValue->SetText(Text);
}

void UARPGLeaf_CharacterText::OnCharacterChosen(const FString& CharacterID)
{
	// get gameinstance then get character info text
	if (CharacterID.IsEmpty()) return;
	if (UPlayerGameInstance* GI = Cast<UPlayerGameInstance>(GetWorld()->GetGameInstance()))
	{
		FCharacterData CharData = GI->GetCharacterDataByID(CharacterID);

		// if text attribute has value
		// then Text_AttributeValue bCollapse is false
        // else is true
		auto ModifierArray = CharData.GetCharacterManifest()->CharacterAttributes;
		auto FoundModifier = ModifierArray.FindByPredicate([this](const FCharacterAttributeModifier& Modifier)
		{
			return Modifier.AttributeName.EqualTo(AttributeNameToQuery);
		});

		if (FoundModifier)
		{
			SetText_Value(FText::AsNumber(FoundModifier->AttributeValue),false);
		}
		else
		{
			SetText_Value(FText(),true);

			if (AttributeName.EqualTo(FText::FromString("CharacterName")))
			{
				SetText_Name(CharData.GetCharacterManifest()->CharacterName,false);
			}
		}
	}
	
}
