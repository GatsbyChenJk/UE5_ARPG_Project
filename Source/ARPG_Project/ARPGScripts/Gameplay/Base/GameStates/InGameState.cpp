// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameState.h"

// #include "ARPGScripts/Gameplay/Base/PlayerGameInstance.h"
// #include "ARPGScripts/Gameplay/GameMap/EscapePointGenerator.h"
// #include "ARPGScripts/Gameplay/GameMap/MapPointManager.h"
//
// void AInGameState::Server_SpawnEscapePoint_Implementation()
// {
// 	if (UPlayerGameInstance* GameInstance = Cast<UPlayerGameInstance>(GetGameInstance()))
// 	{
// 		// for debug use
// 		// test map M03
// 		FMapManifest Manifest = GameInstance->GetMapManifestByID(FString("M04"));
// 		//------------
// 		if (auto MapManager = GameInstance->MapManager)
// 		{
// 			if (Manifest.EscapePoints.Num() > 0)
// 			{
// 				MapManager->InitPointManager(Manifest);
// 				FTransform PortalTransform = MapManager->GetRandomEscapePointTransform();
// 				if (auto ActorClass = MapManager->GetEscapePointGenerator()->GetEscapePointActor())
// 				{
// 					//TODO:Fix spawn nothing problem
// 					FActorSpawnParameters SpawnParams;
// 					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding; // 碰撞处理
// 					SpawnParams.ObjectFlags |= RF_Transient; // 临时Actor，不保存到关卡
// 					SpawnParams.Owner = this; // 设置Owner为GameMode，防止被GC回收
// 					SpawnParams.Name = FName("EscapePortal_01");
// 					AActor* PortalActor = GetWorld()->SpawnActor(ActorClass,&PortalTransform,SpawnParams);
// 				}
// 			}
// 			
// 		}
// 	}
// }
