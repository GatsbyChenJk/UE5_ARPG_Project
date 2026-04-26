// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterManager.h"

#include "GameFramework/Character.h"

void UCharacterManager::RegisterCharacter(ACharacter* Character)
{
	if (IsValid(Character) && !RegisteredCharacters.Contains(Character))
	{
		RegisteredCharacters.Add(Character);
	}
}

void UCharacterManager::UnRegisterCharacter(ACharacter* Character)
{
	if (IsValid(Character) && RegisteredCharacters.Contains(Character))
	{
		RegisteredCharacters.Remove(Character);
	}
}

ACharacter* UCharacterManager::GetPlayerByIndex(int32 CharIndex)
{
	if (RegisteredCharacters.Num()-1 >= CharIndex)
	{
		return RegisteredCharacters[CharIndex];
	}
	return nullptr;
}
