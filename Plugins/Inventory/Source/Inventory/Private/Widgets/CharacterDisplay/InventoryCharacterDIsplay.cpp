// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CharacterDisplay/InventoryCharacterDIsplay.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Equipment/ProxyMesh/InventoryProxyMesh.h"
#include "Kismet/GameplayStatics.h"

FReply UInventoryCharacterDIsplay::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	CurrentPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	LastPosition = CurrentPosition;
	
	bIsDragging = true;
	return FReply::Handled();
}

FReply UInventoryCharacterDIsplay::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bIsDragging = false;
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UInventoryCharacterDIsplay::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bIsDragging = false;
}

void UInventoryCharacterDIsplay::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AInventoryProxyMesh::StaticClass(), Actors);

	if (!Actors.IsValidIndex(0)) return;

	AInventoryProxyMesh* ProxyMesh = Cast<AInventoryProxyMesh>(Actors[0]);
	if (!IsValid(ProxyMesh)) return;

	Mesh = ProxyMesh->GetMesh();
}

void UInventoryCharacterDIsplay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsDragging) return;

	LastPosition = CurrentPosition;
	CurrentPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());

	const float HorizontalDelta = LastPosition.X - CurrentPosition.X;

	if (!Mesh.IsValid()) return;
	Mesh->AddRelativeRotation(FRotator(0.f, HorizontalDelta, 0.f));
}
