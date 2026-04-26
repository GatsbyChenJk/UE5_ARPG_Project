# ARPG 对象池系统使用指南

## 1. 快速开始

### 1.1 创建池配置

在 Content Browser 中右键 -> `Miscellaneous` -> `Data Asset` -> 选择 `ObjectPoolConfig`。

```cpp
// ObjectPoolConfig.h
USTRUCT(BlueprintType)
struct FPoolConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 PoolSize = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxPoolSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bAutoPrewarm = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bNetworkReplicated = true;
};
```

![配置示例](建议截图说明：在 DataAsset 中添加多个 FPoolConfig 条目)

### 1.2 让 Actor 支持池化

**C++ 方式：**

```cpp
// MyProjectile.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoolableActor.h"
#include "MyProjectile.generated.h"

UCLASS()
class ARPG_PROJECT_API AMyProjectile : public AActor, public IPoolableActor
{
    GENERATED_BODY()

public:
    AMyProjectile();

    // IPoolableActor 接口
    virtual void OnActivateFromPool_Implementation() override;
    virtual void OnReturnToPool_Implementation() override;
    virtual bool IsActiveInPool_Implementation() const override;

    UPROPERTY(Replicated)
    bool bIsPooledActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ProjectileSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float LifeTime = 5.0f;

    FTimerHandle LifeTimerHandle;
};
```

```cpp
// MyProjectile.cpp
#include "MyProjectile.h"
#include "ARPG_Project/ARPGScripts/Gameplay/Base/ARPGObjectPoolSystem/PoolSubsystem.h"

AMyProjectile::AMyProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void AMyProjectile::OnActivateFromPool_Implementation()
{
    // 重置状态
    bIsPooledActive = true;
    Velocity = GetActorForwardVector() * ProjectileSpeed;

    // 启动生命周期计时器
    GetWorldTimerManager().SetTimer(LifeTimerHandle, this, &AMyProjectile::OnLifeTimeExpired, LifeTime, false);
}

void AMyProjectile::OnReturnToPool_Implementation()
{
    // 清理状态
    bIsPooledActive = false;
    Velocity = FVector::ZeroVector;

    // 清除计时器
    GetWorldTimerManager().ClearTimer(LifeTimerHandle);
}

bool AMyProjectile::IsActiveInPool_Implementation() const
{
    return bIsPooledActive;
}

void AMyProjectile::OnLifeTimeExpired()
{
    // 自动释放回池
    if (UPoolSubsystem* PoolSub = GetGameInstance()->GetSubsystem<UPoolSubsystem>())
    {
        PoolSub->ReleaseActor(this, this);
    }
}
```

---

## 2. C++ 使用示例

### 2.1 基础请求与释放

```cpp
#include "ARPG_Project/ARPGScripts/Gameplay/Base/ARPGObjectPoolSystem/PoolSubsystem.h"

void AMyWeapon::FireProjectile(const FTransform& SpawnTransform)
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UPoolSubsystem* PoolSub = GI->GetSubsystem<UPoolSubsystem>();
    if (!PoolSub) return;

    // 请求一个子弹
    AActor* Projectile = PoolSub->RequestActor(
        this,                           // WorldContext
        AMyProjectile::StaticClass(),   // ActorClass
        SpawnTransform,                 // Transform
        this,                           // Owner
        Cast<APawn>(GetOwner())         // Instigator
    );

    if (AMyProjectile* MyProj = Cast<AMyProjectile>(Projectile))
    {
        // 可以在这里设置额外参数
        MyProj->ProjectileSpeed = 1500.0f;
    }
}
```

### 2.2 预热池

```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 只在服务器预热
    if (GetNetMode() == NM_Client) return;

    if (UPoolSubsystem* PoolSub = GetGameInstance()->GetSubsystem<UPoolSubsystem>())
    {
        // 预热 20 个子弹
        PoolSub->PrewarmPool(this, AMyProjectile::StaticClass(), 20);

        // 预热 10 个敌人
        PoolSub->PrewarmPool(this, AMyEnemy::StaticClass(), 10);
    }
}
```

### 2.3 查询池状态

```cpp
void UMyDebugWidget::UpdatePoolInfo()
{
    UPoolSubsystem* PoolSub = GetGameInstance()->GetSubsystem<UPoolSubsystem>();
    if (!PoolSub) return;

    int32 Available = PoolSub->GetPoolAvailableCount(this, AMyProjectile::StaticClass());
    int32 InUse = PoolSub->GetPoolInUseCount(this, AMyProjectile::StaticClass());
    int32 Total = PoolSub->GetPoolTotalCount(this, AMyProjectile::StaticClass());

    UE_LOG(LogTemp, Log, TEXT("Projectile Pool - Available: %d, InUse: %d, Total: %d"),
        Available, InUse, Total);
}
```

### 2.4 监听池事件

```cpp
void AMyManager::BeginPlay()
{
    Super::BeginPlay();

    if (UPoolSubsystem* PoolSub = GetGameInstance()->GetSubsystem<UPoolSubsystem>())
    {
        PoolSub->OnActorRequested.AddDynamic(this, &AMyManager::OnActorRequested);
        PoolSub->OnActorReleased.AddDynamic(this, &AMyManager::OnActorReleased);
    }
}

void AMyManager::OnActorRequested(AActor* Actor)
{
    UE_LOG(LogTemp, Log, TEXT("Actor requested from pool: %s"), *Actor->GetName());
}

void AMyManager::OnActorReleased(AActor* Actor)
{
    UE_LOG(LogTemp, Log, TEXT("Actor returned to pool: %s"), *Actor->GetName());
}
```

### 2.5 清空池

```cpp
// 清空某个 World 中特定类的池
PoolSub->ClearPool(this, AMyProjectile::StaticClass());

// 清空某个 World 的所有池
PoolSub->ClearPool(this);

// 清空所有 World 的所有池（GameInstance 级别）
PoolSub->ClearAllPools();
```

---

## 3. Blueprint 使用示例

### 3.1 请求 Actor

在蓝图中调用 `Pool Subsystem` 的 `Request Actor` 节点：

```
[Get Game Instance]
    |
    v
[Get Subsystem (Pool Subsystem)]
    |
    v
[Request Actor]
    - World Context Object: Self
    - Actor Class: BP_MyProjectile
    - Spawn Transform: [Make Transform]
    - Owner: Self
    - Instigator: [Get Player Pawn]
    |
    v
[Spawn Actor From Pool]
```

### 3.2 释放 Actor 回池

```
[Get Game Instance]
    |
    v
[Get Subsystem (Pool Subsystem)]
    |
    v
[Release Actor]
    - World Context Object: Self
    - Actor: [Projectile Reference]
```

### 3.3 在蓝图中实现 IPoolableActor

1. 打开你的 Blueprint Actor
2. 在 `Class Settings` 中点击 `Implemented Interfaces` 的 `Add` 按钮
3. 选择 `PoolableActor`
4. 实现 `On Activate From Pool` 和 `On Return To Pool` 事件

```
Event On Activate From Pool
    |
    +---> [Set IsActive = true]
    +---> [Set Actor Hidden In Game = false]
    +---> [Set Actor Tick Enabled = true]
    +---> [Play Spawn Effect]

Event On Return To Pool
    |
    +---> [Set IsActive = false]
    +---> [Stop All Effects]
    +---> [Clear Timers]
```

---

## 4. 网络同步最佳实践

### 4.1 关键原则

```
+----------------------------------+
|  服务器端（Authority）              |
|  - 调用 RequestActor               |
|  - 调用 ReleaseActor               |
|  - 管理池的生命周期                 |
+----------------------------------+
                |
                | Actor Replication
                v
+----------------------------------+
|  客户端（Simulated/Autonomous）     |
|  - 接收生成的 Actor                 |
|  - 通过 Replicated 属性同步状态     |
|  - 不直接调用池接口                 |
+----------------------------------+
```

### 4.2 推荐的网络设置

```cpp
// 在池化 Actor 的构造函数中
AMyPooledActor::AMyPooledActor()
{
    bReplicates = true;
    SetReplicateMovement(true);
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false; // 池管理 Tick
}
```

### 4.3 状态同步示例

```cpp
UCLASS()
class ARPG_PROJECT_API AMyEnemy : public AARPGBaseCharacter, public IPoolableActor
{
    GENERATED_BODY()

public:
    // 这个属性会在激活时同步给客户端
    UPROPERTY(ReplicatedUsing = OnRep_PoolState)
    FPoolState PoolState;

    UFUNCTION()
    void OnRep_PoolState();
};

void AMyEnemy::OnActivateFromPool_Implementation()
{
    PoolState.bIsActive = true;
    PoolState.SpawnTime = GetWorld()->GetTimeSeconds();
    // 属性复制会自动同步到客户端
}

void AMyEnemy::OnReturnToPool_Implementation()
{
    PoolState.bIsActive = false;
    // 客户端收到后会隐藏/禁用
}

void AMyEnemy::OnRep_PoolState()
{
    // 客户端根据状态更新视觉表现
    SetActorHiddenInGame(!PoolState.bIsActive);
    SetActorTickEnabled(PoolState.bIsActive);
}
```

---

## 5. 常见问题

### Q1: 客户端调用 RequestActor 会发生什么？

```
警告日志: "Actor pooling should only be called on the server"
返回 nullptr
```

**解决方案**：将请求逻辑放在 `HasAuthority()` 分支中，或使用 Server RPC：

```cpp
UFUNCTION(Server, Reliable)
void Server_SpawnProjectile(const FTransform& SpawnTransform);
```

### Q2: Actor 被 Release 后客户端还能看到？

确保 Actor 的 `bReplicates = true`，且 `ForceNetUpdate()` 被正确调用。如果问题持续，检查：

1. Actor 的 `NetUpdateFrequency` 是否过低
2. 客户端的 `OnReturnToPool_Implementation` 中是否正确隐藏了 Actor

### Q3: 切换关卡后池还在吗？

在！`UGameInstanceSubsystem` 的生命周期与 GameInstance 一致，除非调用 `ClearAllPools()` 或 GameInstance 被销毁。

### Q4: 如何查看当前池的调试信息？

```cpp
// 在控制台输入
PoolSubsystem.PrintStats
```

（注：如需此功能，可在 PoolSubsystem 中增加 `PrintPoolStats` 控制台命令）

### Q5: PoolSubsystem 初始化时自动预热怎么配置？

在 `ObjectPoolConfig` 数据资产中，将对应类的 `bAutoPrewarm` 设为 `true`：

```cpp
// PoolSubsystem::Initialize() 中已有支持逻辑
for (const FPoolConfig& Config : PoolConfig->GetAutoPrewarmClasses())
{
    // 预热...
}
```

---

## 6. 性能对比

| 操作 | 普通 Spawn/Destroy | 对象池 |
|---|---|---|
| 首次创建 | 相同 | 相同 |
| 后续请求 | ~1-3ms | ~0.01-0.05ms |
| 内存波动 | 频繁分配/释放 | 稳定复用 |
| GC 压力 | 高 | 极低 |
| 适用场景 | 一次性生成 | 频繁生成/销毁 |

---

## 7. 扩展建议

### 7.1 添加异步预热

对于大量 Actor 的预热，考虑使用 `AsyncTask` 分批创建：

```cpp
// PoolSubsystem.cpp
void UPoolSubsystem::PrewarmPoolAsync(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, int32 Count)
{
    AsyncTask(ENamedThreads::GameThread, [this, WorldContextObject, ActorClass, Count]()
    {
        PrewarmPool(WorldContextObject, ActorClass, Count);
    });
}
```

### 7.2 添加池优先级

对于不同类型的对象，可以按优先级分配池容量：

```cpp
UENUM()
enum class EPoolPriority : uint8
{
    Critical,   // 玩家武器、主角技能
    High,       // 精英敌人
    Normal,     // 普通敌人、普通子弹
    Low         // 装饰性特效
};
```

### 7.3 添加自动伸缩

根据使用率动态调整池大小：

```cpp
void UPoolSubsystem::AutoScalePool(UWorld* World, TSubclassOf<AActor> ActorClass)
{
    FObjectPool& Pool = WorldPools[World][ActorClass];
    float UsageRate = (float)Pool.InUseActors.Num() / Pool.MaxPoolSize;

    if (UsageRate > 0.8f)
    {
        // 扩容
        PrewarmPool(World, ActorClass, Pool.MaxPoolSize * 0.2f);
    }
}
```
