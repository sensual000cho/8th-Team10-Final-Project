#include "Field/Actor/CPRockVineActor.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Resource/System/CPObjectPoolSubsystem.h"
#include "Resource/System/CPResourceStateSubsystem.h"

ACPRockVineActor::ACPRockVineActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RockMesh"));
	RockMesh->SetupAttachment(Root);
	
	VineMesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VineMesh0"));
	VineMesh0->SetupAttachment(Root);
	VineMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VineMesh1"));
	VineMesh1->SetupAttachment(Root);
	VineMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VineMesh2"));
	VineMesh2->SetupAttachment(Root);
	VineMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VineMesh3"));
	VineMesh3->SetupAttachment(Root);
	
	HeatFXComponent0 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HeatFX0"));
	HeatFXComponent0->SetupAttachment(VineMesh0);
	HeatFXComponent1 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HeatFX1"));
	HeatFXComponent1->SetupAttachment(VineMesh1);
	HeatFXComponent2 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HeatFX2"));
	HeatFXComponent2->SetupAttachment(VineMesh2);
	HeatFXComponent3 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HeatFX3"));
	HeatFXComponent3->SetupAttachment(VineMesh3);
}

void ACPRockVineActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bIsScaleTransitioning)
	{
		SetActorTickEnabled(false);
		
		 return;
	}
	
	ScaleTransitionElapsed += DeltaSeconds;
	
	const float LinearAlpha = FMath::Clamp(ScaleTransitionElapsed / ScaleTransitionDuration, 0.f, 1.f);
	const float Alpha = LinearAlpha * LinearAlpha * (3.f - 2.f * LinearAlpha);
	const int32 Count = FMath::Min(VineMeshes.Num(), ScaleTransitionTarget.Num());
	
	for (int32 i = 0; i < Count; ++i)
	{
		UStaticMeshComponent* Vine = VineMeshes[i];
		if (!Vine) continue;
		
		const FVector NewScale = FMath::Lerp(ScaleTransitionStart[i], ScaleTransitionTarget[i], Alpha);
		
		Vine->SetRelativeScale3D(NewScale);
	}
	
	if (LinearAlpha >= 1.f)
	{
		bIsScaleTransitioning = false;
		SetActorTickEnabled(false);
	}
}

bool ACPRockVineActor::PotionReaction(const FGameplayTag& EventTag, AActor* InstigatorActor)
{
	return Super::PotionReaction(EventTag, InstigatorActor);
}

void ACPRockVineActor::Initialize(const FCPResourceNodeKey& InNodeKey)
{
	NodeKey = InNodeKey;
	
	ResetRuntimeState();
}

void ACPRockVineActor::OnAcquireFromPool_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	
	ResetRuntimeState();
}

void ACPRockVineActor::OnReleaseToPool_Implementation()
{
	bIsScaleTransitioning = false;
	SetActorTickEnabled(false);
	SetHeatFXActive(false);
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
	
	NodeKey = FCPResourceNodeKey();
}

void ACPRockVineActor::ResetRuntimeState()
{
	bIsScaleTransitioning = false;
	
	ScaleTransitionElapsed = 0.f;
	ScaleTransitionDuration = 0.f;
	
	ScaleTransitionStart.Reset();
	ScaleTransitionTarget.Reset();
	
	SetActorTickEnabled(false);
	SetHeatFXActive(false);
	
	SetOriginalScaleImmediately();
	SetVinesEnabled(true);
}

void ACPRockVineActor::BeginPlay()
{
	Super::BeginPlay();
	
	VineMeshes = {VineMesh0, VineMesh1, VineMesh2, VineMesh3};
	HeatFXComponents = {HeatFXComponent0, HeatFXComponent1, HeatFXComponent2, HeatFXComponent3};
	
	CacheInitialVineState();
	SetHeatFXActive(false);
	BindGameplayTagEvents();
}

void ACPRockVineActor::CacheInitialVineState()
{
	OriginalScales.Reset();
	OriginalCollisionStates.Reset();
	OriginalVisibilityStates.Reset();
	
	for (UStaticMeshComponent* Vine : VineMeshes)
	{
		if (!Vine) continue;
		
		OriginalScales.Add(Vine->GetRelativeScale3D());
		OriginalCollisionStates.Add(Vine->GetCollisionEnabled());
		OriginalVisibilityStates.Add(Vine->IsVisible());
	}
}

void ACPRockVineActor::BindGameplayTagEvents()
{
	if (!AbilitySystemComponent) return;
	
	AbilitySystemComponent->RegisterGameplayTagEvent(
		FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Giant")),
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnGiantStateChanged);
	
	AbilitySystemComponent->RegisterGameplayTagEvent(
		FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Heat")),
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnHeatStateChanged);
	
	AbilitySystemComponent->RegisterGameplayTagEvent(
		FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Respawning")),
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnRespawningStateChanged);
}

void ACPRockVineActor::OnGiantStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		StartScaleTransition(GiantScaleMultiplier, GrowDuration);
		PlayStartSound();
		
		return;
	}
	
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Heat"))))
	{
		return;
	}
	
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Respawning"))))
	{
		return;
	}

	StartScaleTransition(1.f, ShrinkDuration);
}

void ACPRockVineActor::OnHeatStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		SetHeatFXActive(true);
		
		return;
	}
	
	SetHeatFXActive(false);
	
	if (NodeKey.IsValid())
	{
		HandleVineDestroyed();
		
		return;
	}
	
	ApplyRespawnEffect();
}

void ACPRockVineActor::OnRespawningStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		bIsScaleTransitioning = false;
		SetActorTickEnabled(false);
		SetOriginalScaleImmediately();
		SetVinesEnabled(false);
		
		return;
	}
	
	SetOriginalScaleImmediately();
	SetVinesEnabled(true);
}

void ACPRockVineActor::StartScaleTransition(float TargetMultiplier, float Duration)
{
	if (VineMeshes.Num() != OriginalScales.Num()) return;
	
	ScaleTransitionStart.Reset();
	ScaleTransitionTarget.Reset();
	
	for (int32 i = 0; i < VineMeshes.Num(); ++i)
	{
		UStaticMeshComponent* Vine = VineMeshes[i];
		if (!Vine)
		{
			ScaleTransitionStart.Add(FVector::OneVector);
			ScaleTransitionTarget.Add(FVector::OneVector);
			
			continue;
		}
		
		ScaleTransitionStart.Add(Vine->GetRelativeScale3D());
		ScaleTransitionTarget.Add(OriginalScales[i] * TargetMultiplier);
	}
	
	ScaleTransitionElapsed = 0.f;
	ScaleTransitionDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	
	bIsScaleTransitioning = true;
	
	SetActorTickEnabled(true);
}

void ACPRockVineActor::SetOriginalScaleImmediately()
{
	const int32 Count = FMath::Min(VineMeshes.Num(), OriginalScales.Num());
	
	for (int32 i = 0; i < Count; ++i)
	{
		if (!VineMeshes[i]) continue;
		
		VineMeshes[i]->SetRelativeScale3D(OriginalScales[i]);
	}
}

void ACPRockVineActor::SetHeatFXActive(bool bActive)
{
	for (UNiagaraComponent* HeatFX : HeatFXComponents)
	{
		if (!HeatFX) continue;
		
		if (bActive)
		{
			HeatFX->Activate(true);
			StartLoopSound();
		}

		else
		{
			HeatFX->Deactivate();
			StopLoopSound();
		}
	}
}

void ACPRockVineActor::SetVinesEnabled(bool bEnabled)
{
	for (int32 i = 0; i < VineMeshes.Num(); ++i)
	{
		UStaticMeshComponent* Vine = VineMeshes[i];
		if (!Vine) continue;
		
		if (!bEnabled)
		{
			Vine->SetVisibility(false, true);
			Vine->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
			continue;
		}
		
		if (OriginalVisibilityStates.IsValidIndex(i))
		{
			Vine->SetVisibility(OriginalVisibilityStates[i], true);
		}
		
		if (OriginalCollisionStates.IsValidIndex(i))
		{
			Vine->SetCollisionEnabled(OriginalCollisionStates[i]);
		}
	}
}

void ACPRockVineActor::ApplyRespawnEffect()
{
	if (!AbilitySystemComponent) return;
	if (!RespawnEffectClass) return;
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Effect.Respawning"))))
	{
		return;
	}
	
	const UGameplayEffect* EffectCDO = RespawnEffectClass->GetDefaultObject<UGameplayEffect>();
	
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	
	AbilitySystemComponent->ApplyGameplayEffectToSelf(EffectCDO, 1.f, Context);
}

void ACPRockVineActor::HandleVineDestroyed()
{
	if (NodeKey.IsValid())
	{
		UGameInstance* GI = GetGameInstance();
		if (!GI) return;
		
		UCPResourceStateSubsystem* StateSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>();
		if (!StateSubsystem) return;
		
		StateSubsystem->MarkVineCleared(NodeKey);
		
		UCPObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UCPObjectPoolSubsystem>();
		if (!Pool) return;
		
		Pool->ReleaseActor(this);
	}
	
	// 스포너에서 생성되지 않은 경우(오브젝트 풀링X인 경우)
}
