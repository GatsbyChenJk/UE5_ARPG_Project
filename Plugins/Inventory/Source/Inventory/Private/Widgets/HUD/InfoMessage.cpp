// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/InfoMessage.h"

#include "Components/TextBlock.h"

void UInfoMessage::SetMessage(const FText& Msg)
{
	Text_Message->SetText(Msg);

	if (!bIsMessageActive)
	{
		MessageShow();
	}
	bIsMessageActive = true;

	GetWorld()->GetTimerManager().SetTimer(MessageTimer,[this]()
	{
		MessageHide();
		bIsMessageActive = false;
	},MessageLifeTime,false);
}
