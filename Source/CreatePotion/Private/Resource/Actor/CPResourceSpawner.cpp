#include "Resource/Actor/CPResourceSpawner.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Resource/Actor/CPResourceNodeActor.h"
#include "Resource/System/CPResourceStateSubsystem.h"
#include "Data/CPResourceDefinition.h"

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
	
	if (ResourceDefinition.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : Resource definition is null"), *GetName());
		
		return;
	}
	
	LoadedResourceDefinition = ResourceDefinition.LoadSynchronous();
	if (!LoadedResourceDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : Failed to load ResourceDefinition"), *GetName());
		
		return;
	}
	
	SpawnAllSlots();
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
	
	const FTransform SpawnTransform = CalculateSpawnTransform(SlotIndex, State.Generation);
	
	ACPResourceNodeActor* ResourceActor = GetWorld()->SpawnActor<ACPResourceNodeActor>(
		ResourceNodeActorClass,
		SpawnTransform
	);
	if (!ResourceActor) return;
	
	ResourceActor->InitializeResource(Key, LoadedResourceDefinition);
}

FCPResourceNodeKey ACPResourceSpawner::MakeNodeKey(int32 SlotIndex) const
{
	FCPResourceNodeKey Key;
	
	Key.LevelId = GetLevelId();
	Key.SpawnerId = SpawnerId;
	Key.SlotIndex = SlotIndex;
	
	return Key;
}

FTransform ACPResourceSpawner::CalculateSpawnTransform(int32 SlotIndex, int32 Generation) const
{
	if (!SpawnArea)
	{
		return GetActorTransform();
	}
	
	FRandomStream RandomStream(MakeSpawnSeed(SlotIndex, Generation));
	
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
		ECC_Visibility,
		QueryParams
	);
	
	if (bHit)
	{
		const FVector GroundNormal = Hit.ImpactNormal.GetSafeNormal();
		
		const float Dot = FMath::Clamp(FVector::DotProduct(FVector::UpVector, GroundNormal), -1.f, 1.f);
		
		const float GroundAngle = FMath::RadiansToDegrees(FMath::Acos(Dot));
		
		const FQuat FullAlignRotation = FQuat::FindBetweenNormals(FVector::UpVector, GroundNormal);
		
		const float AllignAlpha = GroundAngle > KINDA_SMALL_NUMBER ?
		FMath::Min(MaxGroundAlignAngle / GroundAngle, 1.f) : 0.f;
		
		const FQuat LimitedAlignRotation = FQuat::Slerp(FQuat::Identity, FullAlignRotation, AllignAlpha);
		
		const float RandomYaw = RandomStream.FRandRange(0.f, 360.f);
		
		const FQuat FinalRotation = LimitedAlignRotation * LimitedAlignRotation;
		
		return FTransform(FinalRotation, Hit.ImpactPoint);
	}
	
	return FTransform(FRotator::ZeroRotator, BaseLocation);
}

// 레벨 전환 시 위치 보존을 위한 시드 생성 코드
// 채집물은 스포너의 일정 범위 내에 랜덤으로 스폰되고, 스폰된 채집물은 위치를 레벨 전환시에도 보존됨
int32 ACPResourceSpawner::MakeSpawnSeed(int32 SlotIndex, int32 Generation) const
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
	
	return static_cast<int32>(Seed & 0x7FFFFFFF);
}

FName ACPResourceSpawner::GetLevelId() const
{
	return FName(UGameplayStatics::GetCurrentLevelName(this, true));
}

