// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Utils/InventoryWidgetUtils.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Widget.h"

bool UInventoryWidgetUtils::IsWidgetInBounds(const FVector2D& BoundaryPos, const FVector2D& WidgetSize,
	const FVector2D& MousePos)
{
	return MousePos.X >= BoundaryPos.X && MousePos.X <= (BoundaryPos.X + WidgetSize.X)
	&& MousePos.Y >= BoundaryPos.Y && MousePos.Y <= (BoundaryPos.Y + WidgetSize.Y);
}

FVector2D UInventoryWidgetUtils::GetWidgetPosition(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry();
	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(Widget,Geometry,USlateBlueprintLibrary::GetLocalTopLeft(Geometry),PixelPosition,ViewportPosition);
	return  ViewportPosition;
}

FVector2D UInventoryWidgetUtils::GetWidgetSize(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry();
	return Geometry.GetLocalSize();
}

int32 UInventoryWidgetUtils::GetIndexFromPosition(const FIntPoint& Position, const int32 Columns)
{
	return Position.X + Position.Y * Columns;
}

FIntPoint UInventoryWidgetUtils::GetPositionFromIndex(const int32 Index, const int32 Columns)
{
	return FIntPoint(Index % Columns, Index / Columns);
}

FVector2D UInventoryWidgetUtils::GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize,
	const FVector2D& MousePos)
{
	FVector2D ClampedPosition = MousePos;

	// Adjust horizontal position to ensure that the widget stays within the boundary
	if (MousePos.X + WidgetSize.X > Boundary.X) // Widget exceeds the right edge
	{
		ClampedPosition.X = Boundary.X - WidgetSize.X;
	}
	if (MousePos.X < 0.f) // Widget exceeds the left edge
	{
		ClampedPosition.X = 0.f;
	}

	// Adjust vertical position to ensure that the widget stays within the boundary
	if (MousePos.Y + WidgetSize.Y > Boundary.Y) // Widget exceeds the bottom edge
	{
		ClampedPosition.Y = Boundary.Y - WidgetSize.Y;
	}
	if (MousePos.Y < 0.f) // Widget exceeds the top edge
	{
		ClampedPosition.Y = 0.f;
	}
	
	return ClampedPosition;
}
