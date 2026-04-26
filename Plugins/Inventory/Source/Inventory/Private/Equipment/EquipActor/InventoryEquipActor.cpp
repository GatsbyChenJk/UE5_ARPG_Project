// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/EquipActor/InventoryEquipActor.h"


AInventoryEquipActor::AInventoryEquipActor()
{
	PrimaryActorTick.bCanEverTick = false;
	// 默认不复制，由调用者根据需要显式启用
	// ProxyMesh 上的装备预览不需要网络复制
	bReplicates = false;
}


