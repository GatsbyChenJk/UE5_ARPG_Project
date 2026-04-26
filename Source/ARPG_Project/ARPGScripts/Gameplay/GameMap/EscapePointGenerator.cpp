// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapePointGenerator.h"

#include "FMapManifest.h"

TArray<FBasePointData> UEscapePointGenerator::GetValidPoints_Implementation()
{
	TArray<FBasePointData> ValidPoints;
	for (const FBasePointData& BasePoint : AllPoints)
	{
		const FEscapePointData& EvacPoint = static_cast<const FEscapePointData&>(BasePoint);
		// 基础规则：启用状态
		if (!EvacPoint.bIsEnabled) continue;
		// 撤离点专属规则1：是否到开启时间
		if (EvacPoint.EscapeWaitingTime > GameRunTime) continue;
		// 撤离点专属规则2：是否需要道具，且玩家没有
		if (EvacPoint.bNeedEscapeItem && !bPlayerHasEscapeItem) continue;

		ValidPoints.Add(EvacPoint);
	}
	return ValidPoints;
}
