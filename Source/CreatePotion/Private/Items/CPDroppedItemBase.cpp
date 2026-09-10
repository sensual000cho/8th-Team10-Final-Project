#include "Items/CPDroppedItemBase.h"

#include "Components/CPInteractableComponent.h"
#include "Components/CPInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

ACPDroppedItemBase::ACPDroppedItemBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionRoot"));
	SetRootComponent(SphereCollision);
	SphereCollision->SetSphereRadius(20.f);
	
	// 일단 채널 지정전까지는 BlockAll
	SphereCollision->SetCollisionProfileName(TEXT("BlockAll"));
	
	VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
	VisualRoot->SetupAttachment(SphereCollision);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(VisualRoot);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(DroppedScale));
	
	// 드랍 관련 설정
	DropMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("DropMovement"));
	DropMovement->bAutoActivate = false;
	DropMovement->ProjectileGravityScale = 1.5f;
	DropMovement->bShouldBounce = false;
	DropMovement->bRotationFollowsVelocity = false;
	DropMovement->SetUpdatedComponent(SphereCollision);
	
	// 호버링 관련 설정
	HoverTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("HoverTimeline"));
	HoverTimeline->SetLooping(true);
	
	// 회전
	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	RotatingMovement->RotationRate = FRotator(0.f, 30.f, 0.f);
	RotatingMovement->bAutoActivate = false;
}

void ACPDroppedItemBase::HandleInteract(AActor* Interactor)
{
	Super::HandleInteract(Interactor);
	
	UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
	if (!Inventory) return;
	
	Inventory->TryGetItemFromData(ItemData, Amount);
	
	UCPObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UCPObjectPoolSubsystem>();
	if (Pool)
	{
		Pool->ReleaseActor(this);
	}

	else
	{
		Destroy();
	}
}

void ACPDroppedItemBase::OnAcquireFromPool_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	
	Mesh->SetHiddenInGame(false, true);
	Mesh->SetVisibility(true, true);
	
	InteractableComponent->SetInteractability(true);
}

void ACPDroppedItemBase::OnReleaseToPool_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	
	ItemData = nullptr;
	Amount = 0;
	
	if (InteractableComponent)
	{
		InteractableComponent->SetInteractability(false);
	}
	DropMovement->StopMovementImmediately();
	DropMovement->Deactivate();
	
	HoverTimeline->Stop();
	
	RotatingMovement->Deactivate();
	
	VisualRoot->SetRelativeLocation(FVector::ZeroVector);
	
	Mesh->SetStaticMesh(nullptr);
}

void ACPDroppedItemBase::Initialize(UCPForageableItemData* InItemData, int32 InAmount, UStaticMesh* InMesh)
{
	ItemData = InItemData;
	Amount = InAmount;
	Mesh->SetStaticMesh(InMesh);
	
	if (InteractableComponent)
	{
		const bool bValid = ItemData != nullptr && Amount > 0;
		InteractableComponent->SetInteractability(bValid);
		InteractableComponent->SetTargetName(ItemData ? FName(*ItemData->DisplayName.ToString()) : NAME_None);
		InteractableComponent->SetInteractionPrompt(FText::FromString(TEXT("줍기")));
	}
}

void ACPDroppedItemBase::StartDropMotion(const FVector& DropDirection)
{
	FVector HorizontalDirection = DropDirection;
	
	HorizontalDirection.Z = 0.f;
	HorizontalDirection.Normalize();
	
	DropMovement->SetUpdatedComponent(SphereCollision);
	DropMovement->Velocity = HorizontalDirection * DropHorizontalSpeed + FVector::UpVector * DropVerticalSpeed;
	DropMovement->Activate(true);
}

void ACPDroppedItemBase::BeginPlay()
{
	Super::BeginPlay();
	
	DropMovement->OnProjectileStop.AddDynamic(this, &ACPDroppedItemBase::OnDropMotionStopped);
	
	if (HoverCurve)
	{
		FOnTimelineFloat HoverUpdate;
		
		HoverUpdate.BindUFunction(this, FName("UpdateHover"));
		
		HoverTimeline->AddInterpFloat(HoverCurve, HoverUpdate);
		HoverTimeline->SetLooping(true);
	}
}

void ACPDroppedItemBase::OnDropMotionStopped(const FHitResult& ImpactResult)
{
	DropMovement->Deactivate();
	
	StartHover();
}

void ACPDroppedItemBase::StartHover()
{
	if (!HoverTimeline) return;
	if (!HoverCurve) return;
	
	HoverBaseLocation = VisualRoot->GetRelativeLocation();
	HoverBaseLocation.Z += HoverHeight;
	
	VisualRoot->SetRelativeLocation(HoverBaseLocation);
	
	HoverTimeline->PlayFromStart();
	
	RotatingMovement->SetUpdatedComponent(VisualRoot);
	RotatingMovement->Activate();
}

void ACPDroppedItemBase::UpdateHover(float Value)
{
	FVector NewLocation = HoverBaseLocation;
	
	NewLocation.Z += Value * HoverAmplitude;
	
	VisualRoot->SetRelativeLocation(NewLocation);
}
