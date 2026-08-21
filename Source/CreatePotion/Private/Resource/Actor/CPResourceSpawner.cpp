#include "Resource/Actor/CPResourceSpawner.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Resource/Actor/CPResourceNodeActor.h"
#include "Resource/System/CPResourceStateSubsystem.h"
#include "Data/CPResourceDefinition.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

#define GROUND_CHANNEL ECC_GameTraceChannel1
#define SPAWN_BLOCK_CHANNEL ECC_GameTraceChannel2

namespace
{
	// FNV-1a 32bit 상수값
	constexpr uint32 FNV32OffsetBasis = 2166136261u;
	constexpr uint32 FNV32Prime = 16777619u;
}

ACPResourceSpawner::ACPResourceSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	SpawnArea->SetupAttachment(SceneRoot);
	SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACPResourceSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UCPResourceStateSubsystem* StateSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>();
		if (StateSubsystem)
		{
			StateSubsystem->OnResourceNodeHarvested.AddUObject(this, &ACPResourceSpawner::HandleNodeHarvested);
		}
		
		UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>();
		if (TimeSubsystem)
		{
			TimeSubsystem->OnTimeChanged.AddDynamic(this, &ACPResourceSpawner::HandleTimeChanged);
		}
	}
	
	if (SpawnEntries.Num() == 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s : Resource definition is null"), *GetName());
		
		return;
	}
	
	SpawnAllSlots();
}

void ACPResourceSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UCPResourceStateSubsystem* StateSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>();
		if (StateSubsystem)
		{
			StateSubsystem->OnResourceNodeHarvested.RemoveAll(this);
		}
		
		UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>();
		if (TimeSubsystem)
		{
			TimeSubsystem->OnTimeChanged.RemoveDynamic(this, &ACPResourceSpawner::HandleTimeChanged);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void ACPResourceSpawner::PostActorCreated()
{
	Super::PostActorCreated();
	
	if (!SpawnerId.IsValid())
	{
		SpawnerId = FGuid::NewGuid();
	}
}

void ACPResourceSpawner::PostEditImport()
{
	Super::PostEditImport();
	
	SpawnerId = FGuid::NewGuid();
}

void ACPResourceSpawner::HandleNodeHarvested(const FCPResourceNodeKey& Key)
{
	if (Key.LevelId != GetLevelId() || Key.SpawnerId != SpawnerId) return;
	
	PendingRespawnSlots.Add(Key.SlotIndex);
}

void ACPResourceSpawner::HandleTimeChanged(int64 Time)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPResourceStateSubsystem* StateSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>();
	if (!StateSubsystem) return;
	
	TArray<int32> ReadySlots;
	
	for (const int32& SlotIndex : PendingRespawnSlots)
	{
		const FCPResourceNodeKey Key = MakeNodeKey(SlotIndex);
		if (StateSubsystem->IsReady(Key))
		{
			ReadySlots.Add(SlotIndex);
		}
	}
	
	for (const int32& SlotIndex : ReadySlots)
	{
		SpawnSlot(SlotIndex);
		PendingRespawnSlots.Remove(SlotIndex);
	}
}

void ACPResourceSpawner::SpawnAllSlots()
{
	for (int32 SlotIndex = 0; SlotIndex < SpawnCount; ++SlotIndex)
	{
		SpawnSlot(SlotIndex);
	}
}

void ACPResourceSpawner::SpawnSlot(int32 SlotIndex)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	
	UCPResourceStateSubsystem* StateSubsystem = GameInstance->GetSubsystem<UCPResourceStateSubsystem>();
	if (!StateSubsystem) return;
	
	const FCPResourceNodeKey Key = MakeNodeKey(SlotIndex);
	if (!Key.IsValid()) return;
	
	FCPResourceNodeState& State = StateSubsystem->GetOrCreateState(Key);
	
	if (!StateSubsystem->IsReady(Key))
	{
		PendingRespawnSlots.Add(SlotIndex);
		return;
	}
	
	const FCPResourceSpawnEntry* Entry = SelectResource(SlotIndex, State.Generation);
	if (!Entry) return;
	
	UCPResourceDefinition* Definition = Entry->ResourceDefinition;
	if (!Definition) return;
	
	FTransform SpawnTransform;
	if (!TryCalculateSpawnTransform(SlotIndex, State.Generation, SpawnTransform)) return;
	
	UCPObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UCPObjectPoolSubsystem>();
	if (!PoolSubsystem) return;
	
	AActor* AcquiredActor = PoolSubsystem->AcquireActor(ResourceNodeActorClass, SpawnTransform);
	if (!AcquiredActor) return;
	
	ACPResourceNodeActor* ResourceActor = Cast<ACPResourceNodeActor>(AcquiredActor);
	if (!ResourceActor) return;
	
	ResourceActor->InitializeResource(Key, Definition);
	PendingRespawnSlots.Add(SlotIndex);
}

const FCPResourceSpawnEntry* ACPResourceSpawner::SelectResource(int32 SlotIndex, int32 Generation) const
{
	float TotalWeight = 0.f;
	
	for (const FCPResourceSpawnEntry& Entry : SpawnEntries)
	{
		if (Entry.ResourceDefinition && Entry.Weight > 0.f)
		{
			TotalWeight += Entry.Weight;
		}
	}
	
	if (TotalWeight <= 0.f) return nullptr;
	
	FRandomStream RandomStream(MakeSpawnSeed(SlotIndex, Generation, 1));
	
	const float Roll = RandomStream.FRandRange(0.f, TotalWeight);
	float AccumulatedWeight = 0.f;
	
	for (const FCPResourceSpawnEntry& Entry : SpawnEntries)
	{
		if (!Entry.ResourceDefinition || Entry.Weight <= 0.f) continue;
		
		AccumulatedWeight += Entry.Weight;
		
		if (Roll <=	AccumulatedWeight)
		{
			return &Entry;
		}
	}
	
	return nullptr;
}

FCPResourceNodeKey ACPResourceSpawner::MakeNodeKey(int32 SlotIndex) const
{
	FCPResourceNodeKey Key;
	
	Key.LevelId = GetLevelId();
	Key.SpawnerId = SpawnerId;
	Key.SlotIndex = SlotIndex;
	
	return Key;
}

bool ACPResourceSpawner::TryCalculateSpawnTransform(int32 SlotIndex, int32 Generation, FTransform& OutTransform) const
{
	if (!SpawnArea) return false;
	
	FRandomStream RandomStream(MakeSpawnSeed(SlotIndex, Generation, 2));
	
	// 오브젝트 겹침 방지 시도 횟수
	constexpr int32 MaxAttempts = 10;
	
	for (int i = 0; i < MaxAttempts; ++i)
	{
		const FVector BoxExtent = SpawnArea->GetUnscaledBoxExtent();
		const FVector LocalLocation(
			RandomStream.FRandRange(-BoxExtent.X, BoxExtent.X),
			RandomStream.FRandRange(-BoxExtent.Y, BoxExtent.Y),
			0.f
		);
	
		const FVector BaseLocation = SpawnArea->GetComponentTransform().TransformPosition(LocalLocation);
		const FVector Start = BaseLocation + FVector(0.f, 0.f, BoxExtent.Z);
		const FVector End = BaseLocation - FVector(0.f, 0.f, BoxExtent.Z);
	
		FHitResult Hit;
	
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
	
		const bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			Start,
			End,
			GROUND_CHANNEL,
			QueryParams
		);
	
		if (bHit)
		{
			// 오브젝트 겹침 체크
			const bool bBlocked = GetWorld()->OverlapBlockingTestByChannel(
				Hit.ImpactPoint,
				FQuat::Identity,
				SPAWN_BLOCK_CHANNEL,
				FCollisionShape::MakeSphere(50.f),
				QueryParams
			);
			
			if (bBlocked && i < MaxAttempts - 1) continue;
			
			const FVector GroundNormal = Hit.ImpactNormal.GetSafeNormal();
		
			const float Dot = FMath::Clamp(FVector::DotProduct(FVector::UpVector, GroundNormal), -1.f, 1.f);
		
			const float GroundAngle = FMath::RadiansToDegrees(FMath::Acos(Dot));
		
			const FQuat FullAlignRotation = FQuat::FindBetweenNormals(FVector::UpVector, GroundNormal);
		
			const float AllignAlpha = GroundAngle > KINDA_SMALL_NUMBER ?
			FMath::Min(MaxGroundAlignAngle / GroundAngle, 1.f) : 0.f;
		
			const FQuat LimitedAlignRotation = FQuat::Slerp(FQuat::Identity, FullAlignRotation, AllignAlpha);
		
			const float RandomYaw = RandomStream.FRandRange(0.f, 360.f);
		
			const FQuat RandomYawRotation(FVector::UpVector, FMath::DegreesToRadians(RandomYaw));
		
			const FQuat FinalRotation = LimitedAlignRotation * RandomYawRotation;
		
			OutTransform = FTransform(FinalRotation, Hit.ImpactPoint);
			
			return true;
		}
	}
	
	return false;
}

// 레벨 전환 시 위치 보존을 위한 시드 생성 코드
// 채집물은 스포너의 일정 범위 내에 랜덤으로 스폰되고, 스폰된 채집물은 위치를 레벨 전환시에도 보존됨
int32 ACPResourceSpawner::MakeSpawnSeed(int32 SlotIndex, int32 Generation, int32 Salt) const
{
	uint32 Seed = FNV32OffsetBasis;
	
	auto Mix = [&Seed](const int32 Value)
	{
		Seed ^= Value;
		Seed *= FNV32Prime;
	};
	
	Mix(SpawnerId.A);
	Mix(SpawnerId.B);
	Mix(SpawnerId.C);
	Mix(SpawnerId.D);
	
	Mix(static_cast<uint32>(SlotIndex));
	Mix(static_cast<uint32>(Generation));
	Mix(static_cast<uint32>(Salt));
	
	return static_cast<int32>(Seed & 0x7FFFFFFF);
}

FName ACPResourceSpawner::GetLevelId() const
{
	return FName(UGameplayStatics::GetCurrentLevelName(this, true));
}

