# ARPGAbilitySystemComponent — 技术文档

> 所在目录：`Source/ARPG_Project/ARPGScripts/Gameplay/AbilitySystem/`

---

## 目录

1. [模块概述](#1-模块概述)
2. [数据结构](#2-数据结构)
3. [功能一：多骨骼网格蒙太奇同步](#3-功能一多骨骼网格蒙太奇同步)
4. [功能二：Socket 位置同步](#4-功能二socket-位置同步)
5. [网络角色与数据流向](#5-网络角色与数据流向)
6. [集成步骤](#6-集成步骤)
7. [API 参考](#7-api-参考)
8. [控制台变量](#8-控制台变量)
9. [常见问题与注意事项](#9-常见问题与注意事项)

---

## 1. 模块概述

`UARPGAbilitySystemComponent` 继承自 `UAbilitySystemComponent`，是本项目的自定义 GAS 核心组件。它在引擎原生 ASC 的基础上新增了两个独立功能：

| 功能 | 解决的问题 | 技术方案 |
|---|---|---|
| 多骨骼网格蒙太奇同步 | 引擎默认 ASC 只能同步单个骨骼网格（角色 Mesh）的蒙太奇；武器有独立 Mesh 时无法同步 | 为每个骨骼网格维护独立的本地状态和复制状态结构体数组，分别驱动 |
| Socket 位置同步 | 服务端命中检测使用 Socket 坐标，Simulated Proxy 客户端拿不到权威的 World-space 坐标 | 服务端每帧采样指定 Socket，通过 `UPROPERTY(ReplicatedUsing=...)` 推送到所有客户端 |

**设计来源：** 蒙太奇同步部分移植自 [GASShooter（Dan Kestranek）](https://github.com/donovankyle/GASShooter) 中的 `UGSAbilitySystemComponent`，并根据 UE5 API 进行了适配（`SequenceLength` → `GetPlayLength()`）。

**宿主对象：** `AAInGamePlayerState`，通过 `IAbilitySystemInterface` 暴露给所有需要它的系统。

---

## 2. 数据结构

### 2.1 蒙太奇相关结构体

```
FARPGLocalAnimMontageForMesh          FARPGRepAnimMontageForMesh
─────────────────────────────         ──────────────────────────────────────────
Mesh : USkeletalMeshComponent*        Mesh : USkeletalMeshComponent*
LocalMontageInfo                      RepMontageInfo
  ├─ AnimMontage*                       ├─ AnimMontage*
  ├─ AnimatingAbility*                  ├─ PlayRate
  └─ PlayBit (toggle)                   ├─ Position
                                        ├─ BlendTime
不参与网络复制                            ├─ IsStopped
服务端 + 自治代理（Owning Client）持有     ├─ NextSectionID
                                        ├─ ForcePlayBit (toggle, 触发重播)
                                        └─ SkipPositionCorrection

                                      ReplicatedUsing=OnRep_ReplicatedAnimMontageForMesh
                                      服务端 → 所有客户端
```

两个数组均以 `USkeletalMeshComponent*` 为 Key，按需动态添加条目（`GetLocalAnimMontageInfoForMesh` / `GetRepAnimMontageInfoForMesh`）。

### 2.2 Socket 复制结构体

```
FARPGRepSocketEntry
───────────────────────────────
SocketName  : FName             // Socket 名称
Location    : FVector           // World-space 位置（cm）
Rotation    : FRotator          // World-space 旋转
bIsWeaponSocket : bool          // true = 武器 Mesh，false = 角色 Mesh
```

`ToTransform()` 方法将结构体直接转换为 `FTransform`，供调用方使用。

---

## 3. 功能一：多骨骼网格蒙太奇同步

### 3.1 逻辑流程

#### 服务端播放蒙太奇

```
Ability（Server）
  └─ PlayMontageForMesh(InMesh, Montage, PlayRate)
       ├─ AnimInstance->Montage_Play()
       ├─ 写入 LocalAnimMontageInfoForMeshes[InMesh]
       │    └─ AnimMontage, AnimatingAbility, PlayBit(toggle)
       ├─ 写入 RepAnimMontageInfoForMeshes[InMesh]
       │    └─ AnimMontage, ForcePlayBit(toggle)
       ├─ AnimMontage_UpdateReplicatedDataForMesh()
       │    └─ 填充 RepMontageInfo: PlayRate, Position, BlendTime, NextSectionID, IsStopped
       └─ AvatarActor->ForceNetUpdate()  ←── 立即推送复制
```

#### Tick 维护（服务端）

```
TickComponent (Server only)
  ├─ 遍历 LocalAnimMontageInfoForMeshes
  │    └─ AnimMontage_UpdateReplicatedDataForMesh(Mesh)  // 保持 Position/PlayRate 同步
  └─ 若 Socket 追踪活跃 → Server_UpdateSocketReplicationData()
```

#### 客户端接收（Simulated Proxy）

```
OnRep_ReplicatedAnimMontageForMesh
  └─ 遍历 RepAnimMontageInfoForMeshes
       ├─ ForcePlayBit 变化 → PlayMontageSimulatedForMesh()  // 首次或强制重播
       ├─ 同步 PlayRate
       ├─ IsStopped == true → CurrentMontageStopForMesh()
       └─ 位置/Section 偏差修正
            ├─ Section 不匹配 → Montage_SetNextSection / Montage_SetPosition
            └─ 位置误差 > MontageErrorThreshold → UpdateWeight + 快进到服务端位置
```

#### 自治代理（Owning Client）预测播放

```
PlayMontageForMesh（Client Predictive）
  ├─ AnimInstance->Montage_Play()  // 立即本地播放
  └─ PredictionKey.NewRejectedDelegate()
       └─ 若服务端拒绝 → OnPredictiveMontageRejectedForMesh()
            └─ AnimInstance->Montage_Stop(0.25f)
```

### 3.2 Section / PlayRate 修改流程

当自治代理在本地改变 Section 或 PlayRate 时：

```
Client                          Server
  │                               │
  ├─ Montage_SetNextSection()     │
  ├─ ServerCurrentMontageSetNextSectionNameForMesh() ──RPC──►
  │                               ├─ 应用到服务端本地
  │                               ├─ 校验 Position 一致性
  │                               └─ AnimMontage_UpdateReplicatedDataForMesh()
  │                                    └─ 推送到所有 Simulated Proxy
```

---

## 4. 功能二：Socket 位置同步

### 4.1 逻辑流程

#### 数据写入（服务端 Tick）

```
Server_UpdateSocketReplicationData()
  ├─ 采样角色 SkeletalMesh 的 TrackedCharacterSockets
  │    └─ SkelMesh->GetSocketTransform(SocketName) → FARPGRepSocketEntry(bIsWeaponSocket=false)
  ├─ 采样武器 StaticMesh 的 TrackedWeaponSockets（若 TrackedWeaponActor 有效）
  │    └─ WeaponMesh->GetSocketTransform(SocketName) → FARPGRepSocketEntry(bIsWeaponSocket=true)
  ├─ 比较新旧数据（O(n)，Socket 数量通常 1-4 个）
  └─ 若有变化 → RepSocketData = NewData + AvatarActor->ForceNetUpdate()
```

#### 客户端接收

```
OnRep_ReplicatedSocketData()
  └─ 空实现（poll 模式）
       └─ 若需要 push 通知 → 在此绑定 Delegate
```

#### 数据查询（任意网络角色）

```
GetReplicatedSocketTransform(SocketName, bFromWeaponMesh, bFound)
  ├─ Server / Autonomous Proxy
  │    └─ 直接查询 Live Mesh（最高精度）
  │         ├─ bFromWeaponMesh=true  → TrackedWeaponActor->GetCachedMeshComponent()->GetSocketTransform()
  │         └─ bFromWeaponMesh=false → AvatarActor->GetMesh()->GetSocketTransform()
  └─ Simulated Proxy
       └─ 遍历 RepSocketData，按 SocketName + bIsWeaponSocket 匹配
```

### 4.2 武器切换时的生命周期

```
装备武器（服务端 UEquipmentComponent）
  ├─ 生成 WeaponActor 并 AttachToComponent
  ├─ ASC->SetTrackedWeaponActor(WeaponActor)
  ├─ ASC->SetTrackedWeaponSocketNames(WeaponConfig.DetectSocketNames)
  └─ ASC->SetTrackedCharacterSocketNames(WeaponConfig.DetectSocketNames)

卸下武器（服务端）
  ├─ ASC->ClearTrackedWeaponActor()      // 清除引用，同时从 RepSocketData 移除武器条目
  └─ ASC->SetTrackedCharacterSocketNames({})  // 停止角色 Socket 采样
```

---

## 5. 网络角色与数据流向

```
┌────────────────────────────────────────────────────────────────────┐
│                          SERVER                                    │
│                                                                    │
│  LocalAnimMontageInfoForMeshes[]  ← Ability播放/停止/Section变更   │
│         │                                                          │
│         ▼ AnimMontage_UpdateReplicatedDataForMesh()                │
│  RepAnimMontageInfoForMeshes[]    ──── REPLICATED ──────────────►  │
│                                                                    │
│  TrackedCharacterSockets                                           │
│  TrackedWeaponSockets     → Server_UpdateSocketReplicationData()   │
│                                    │                               │
│                            RepSocketData[]  ── REPLICATED ───────► │
└────────────────────────────────────────────────────────────────────┘

           ▼ OnRep_ReplicatedAnimMontageForMesh          ▼ OnRep_ReplicatedSocketData
┌──────────────────────────┐              ┌──────────────────────────┐
│  AUTONOMOUS PROXY        │              │  SIMULATED PROXY         │
│  (Owning Client)         │              │                          │
│                          │              │  PlayMontageSimulated()  │
│  - 本地预测播放已完成      │              │  ← 由 OnRep 触发         │
│  - OnRep 仅做 PlayRate    │              │                          │
│    和 Position 校正       │              │  GetReplicatedSocket()   │
│                          │              │  ← 从 RepSocketData 读取 │
└──────────────────────────┘              └──────────────────────────┘
```

**关键规则：**
- `LocalAnimMontageInfoForMeshes` 永不复制，仅在本地（服务端或自治代理）存活。
- `RepAnimMontageInfoForMeshes` 是唯一跨网络传输蒙太奇状态的载体。
- Socket 数据始终由服务端写入，客户端只读。
- `ForceNetUpdate()` 在关键状态变更后立即调用，避免等待下一个网络帧。

---

## 6. 集成步骤

### 6.1 PlayerState 接入

`AAInGamePlayerState` 已更新，使用 `UARPGAbilitySystemComponent` 替代了原来的 `UAbilitySystemComponent`：

```cpp
// AAInGamePlayerState.h
#include "ARPGScripts/Gameplay/AbilitySystem/ARPGAbilitySystemComponent.h"

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UARPGAbilitySystemComponent> AbilitySystemComponent;

// 新增类型安全访问器
UFUNCTION(BlueprintCallable)
UARPGAbilitySystemComponent* GetARPGAbilitySystemComponent() const;
```

原有的 `GetAbilitySystemComponent()` (IAbilitySystemInterface 重写) 返回基类指针，不影响现有调用者。

### 6.2 装备系统接入

在 `UEquipmentComponent::Server_InitCharacterEquipmentAbility()` 中，武器 Attach 完成后：

```cpp
// 获取 ASC
UARPGAbilitySystemComponent* ARPGAsc = Cast<UARPGAbilitySystemComponent>(
    UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()));

if (ARPGAsc)
{
    // 注册武器 Actor
    ARPGAsc->SetTrackedWeaponActor(CurrentWeapon);

    // 注册需要追踪的 Socket 名（来自 FWeaponManifest 或武器配置）
    ARPGAsc->SetTrackedWeaponSocketNames(WeaponConfig.DetectSocketNames);
    ARPGAsc->SetTrackedCharacterSocketNames(WeaponConfig.DetectSocketNames);
}
```

卸下武器时：

```cpp
if (ARPGAsc)
{
    ARPGAsc->ClearTrackedWeaponActor();
    ARPGAsc->SetTrackedCharacterSocketNames({});
    ARPGAsc->SetTrackedWeaponSocketNames({});
}
```

### 6.3 命中检测系统接入

`UCharacterAttackComponent::DetectSockets()` 中，将硬编码的 Mesh 直接查询替换为：

```cpp
UARPGAbilitySystemComponent* ARPGAsc = ...;

bool bFound = false;
FTransform WeaponTipTransform = ARPGAsc->GetReplicatedSocketTransform(
    FName("weapon_tip"),
    /*bFromWeaponMesh=*/true,
    bFound);

if (bFound)
{
    // 使用 WeaponTipTransform.GetLocation() 做 SweepTest
}
```

### 6.4 在 Ability 中播放蒙太奇

```cpp
// 在 UGameplayAbility::ActivateAbility() 中
UARPGAbilitySystemComponent* ASC = Cast<UARPGAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
USkeletalMeshComponent* CharacterMesh = MyCharacter->GetMesh();

// 在角色主 Mesh 上播放（全平台同步）
float Duration = ASC->PlayMontageForMesh(
    this,                       // 当前 Ability
    CharacterMesh,              // 目标骨骼 Mesh
    CurrentActivationInfo,
    AttackMontage,
    1.0f,                       // PlayRate
    FName("Start"),             // StartSection（可选）
    true                        // bReplicateMontage
);

// 停止
ASC->CurrentMontageStopForMesh(CharacterMesh);

// 跳 Section（客户端调用时自动通过 Server RPC 同步）
ASC->CurrentMontageJumpToSectionForMesh(CharacterMesh, FName("Loop"));
```

---

## 7. API 参考

### 蒙太奇 API

| 函数 | 说明 | 调用方 |
|---|---|---|
| `PlayMontageForMesh(Ability, Mesh, ActivationInfo, Montage, Rate, Section, bReplicate)` | 在指定 Mesh 上播放蒙太奇，服务端会复制到 Simulated Proxy | Server / Owning Client（预测） |
| `PlayMontageSimulatedForMesh(Mesh, Montage, Rate, Section)` | 不带网络逻辑的本地播放，供 OnRep 内部调用 | 内部 / Simulated Proxy |
| `CurrentMontageStopForMesh(Mesh, BlendOutTime)` | 停止指定 Mesh 上的当前蒙太奇 | 任意 |
| `StopAllCurrentMontages(BlendOutTime)` | 停止所有 Mesh 上的蒙太奇 | 任意 |
| `CurrentMontageJumpToSectionForMesh(Mesh, SectionName)` | 跳到指定 Section（客户端自动发 Server RPC） | 任意 |
| `CurrentMontageSetNextSectionNameForMesh(Mesh, From, To)` | 设置下一 Section（客户端自动发 Server RPC） | 任意 |
| `CurrentMontageSetPlayRateForMesh(Mesh, Rate)` | 修改播放速率（客户端自动发 Server RPC） | 任意 |
| `GetCurrentMontageForMesh(Mesh)` | 获取当前正在播放的 Montage | 任意 |
| `GetCurrentMontageSectionNameForMesh(Mesh)` | 获取当前 Section 名 | 任意 |
| `GetCurrentMontageSectionTimeLeftForMesh(Mesh)` | 获取当前 Section 剩余时长（秒） | 任意 |
| `IsAnimatingAbilityForAnyMesh(Ability)` | 检查该 Ability 是否在任意 Mesh 上正在播放动画 | 任意 |

### Socket API

| 函数 | 说明 | 调用方 |
|---|---|---|
| `SetTrackedCharacterSocketNames(Names)` | 注册角色 Mesh 上需每帧采样的 Socket 列表 | Server |
| `SetTrackedWeaponSocketNames(Names)` | 注册武器 Mesh 上需每帧采样的 Socket 列表 | Server |
| `SetTrackedWeaponActor(Weapon)` | 设置当前追踪的武器 Actor | Server |
| `ClearTrackedWeaponActor()` | 清除武器追踪并从复制数组移除武器 Socket 条目 | Server |
| `GetReplicatedSocketTransform(SocketName, bFromWeaponMesh, bFound)` | 查询指定 Socket 的 World-space Transform；Server/Owning Client 查询 Live Mesh，Simulated Proxy 返回最近一次复制快照 | 任意 |

---

## 8. 控制台变量

| 变量名 | 默认值 | 说明 |
|---|---|---|
| `ARPG.replay.MontageErrorThreshold` | `0.5` | Replay 回放时蒙太奇位置误差阈值（秒）。正常联机时固定使用 `0.1f`。超过阈值时客户端会快进到服务端位置。 |

---

## 9. 常见问题与注意事项

### Q：Simulated Proxy 上 Socket 数据有一帧延迟，命中检测会不准确吗？

服务端和自治代理上 `GetReplicatedSocketTransform()` 直接查询 Live Mesh，不走复制快照。**命中检测的权威判断始终在服务端执行**，Simulated Proxy 的快照仅用于视觉效果（特效、UI）。因此一帧延迟对游戏逻辑没有影响。

### Q：如何为武器的独立 SkeletalMeshComponent（非 StaticMesh）添加 Socket 同步支持？

当前实现假设武器使用 `UStaticMeshComponent`（通过 `AARPGBaseWeapon::GetCachedMeshComponent()`）。如果武器使用 `USkeletalMeshComponent`，需要：
1. 在 `AARPGBaseWeapon` 中添加返回 `USkeletalMeshComponent*` 的接口方法。
2. 在 `Server_UpdateSocketReplicationData()` 中分支处理。

### Q：武器卸下后客户端上 RepSocketData 中的旧武器 Socket 什么时候清理？

`ClearTrackedWeaponActor()` 会立即调用 `RepSocketData.RemoveAll(bIsWeaponSocket)` 并触发 `ForceNetUpdate()`，客户端在下一个网络帧收到新的 `RepSocketData`（不含武器条目）后自动更新。

### Q：多个 Ability 同时在同一 Mesh 上播放蒙太奇会怎样？

`PlayMontageForMesh` 会直接覆盖 `LocalMontageInfoForMesh` 中的条目，行为与引擎原生 `PlayMontage` 一致——新蒙太奇会打断旧蒙太奇。如需混合，应使用不同的 Mesh 实例或蒙太奇 Slot。

### Q：`GetShouldTick()` 什么时候返回 false？

两个条件均为假时停止 Tick：
- 没有任何 Mesh 正在播放蒙太奇（服务端视角）
- `TrackedCharacterSockets` 和 `TrackedWeaponSockets` 均为空

这意味着角色静止且无武器 Socket 追踪时，组件不消耗 Tick 性能。

### Q：能在 Blueprint 中直接调用吗？

- `SetTrackedCharacterSocketNames`、`SetTrackedWeaponSocketNames`、`SetTrackedWeaponActor`、`ClearTrackedWeaponActor`、`GetReplicatedSocketTransform` 均标记了 `BlueprintCallable` / `BlueprintPure`，可在 BP 中调用。
- 蒙太奇 API 目前仅暴露给 C++。若需要 BP 访问，在 `UGameplayAbilityHelper`（现有的 `UBlueprintFunctionLibrary`）中添加包装函数即可。
