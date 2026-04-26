// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/HighlightableMeshComp.h"

void UHighlightableMeshComp::Highlight_Implementation()
{
	SetOverlayMaterial(HighlightMaterial);
}

void UHighlightableMeshComp::UnHighlight_Implementation()
{
	SetOverlayMaterial(nullptr);
}
