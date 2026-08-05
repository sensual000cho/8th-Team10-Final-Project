// CPInteractionComponent.cpp


#include "Character/CPInteractionComponent.h"
#include "GameCore/Interface/CPInteractable.h"
#include "Camera/CameraComponent.h"
#include "Character/CPCharacter.h"
#include "HAL/IConsoleManager.h"

// 에디터에서 콘솔창에 cp.Debug.Interaction를 입력해서 Debug On/Off 가능
// cp.Debug.Interaction 1 --> Debug On
// cp.Debug.Interaction 0 --> Debug Off 
static TAutoConsoleVariable<bool> CVarDebugInteraction(
	TEXT("cp.Debug.Interaction"),
	false,
	TEXT("Draw interaction trace debug lines\n")
	TEXT("0: Disabled\n")
	TEXT("1: Enabled"),
	ECVF_Cheat
);

void UCPInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ACPCharacter* Character = Cast<ACPCharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	Camera = Character->GetFollowCamera();
	
	// Trace하는 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(
		TraceTimerHandle,
		this,
		&UCPInteractionComponent::PerformTrace,
		0.15f,
		true
	);
}

void UCPInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void UCPInteractionComponent::PerformTrace()
{
	if (!Camera || !GetOwner())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	// 카메라 기준 LineTrace
	constexpr float CameraTraceDistance = 5000.f;

	const FVector CameraStart = Camera->GetComponentLocation();
	const FVector CameraEnd = CameraStart + Camera->GetForwardVector() * CameraTraceDistance;

	FHitResult CameraHit;

	const bool bCameraHit = World->LineTraceSingleByChannel(
		CameraHit,
		CameraStart,
		CameraEnd,
		ECC_Visibility,
		QueryParams
	);

	const FVector AimPoint = bCameraHit ? CameraHit.ImpactPoint : CameraEnd;
	
	// 캐릭터에서 조준 지점을 향해서 Trace
	
	constexpr float TraceHeightOffset = 70.f;
	constexpr float SphereRadius = 50.f;

	const FVector CharacterLocation = GetOwner()->GetActorLocation();
	const FVector SphereStart = CharacterLocation + FVector(0.f, 0.f, TraceHeightOffset);
	FVector DirectionToAim = AimPoint - SphereStart;

	if (DirectionToAim.IsNearlyZero())
	{
		ClearCurrentTarget();
		return;
	}

	DirectionToAim.Normalize();
	
	// 실제 상호작용 거리는 TraceDistance
	const FVector SphereEnd =
		SphereStart + DirectionToAim * TraceDistance;

	FHitResult InteractionHit;

	const bool bInteractionHit = World->SweepSingleByChannel(
		InteractionHit,
		SphereStart,
		SphereEnd,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(SphereRadius),
		QueryParams
	);
	
#if ENABLE_DRAW_DEBUG
	if (CVarDebugInteraction.GetValueOnGameThread())
	{
		// 카메라 Line Trace
		DrawDebugLine(
			World,
			CameraStart,
			AimPoint,
			bCameraHit ? FColor::Blue : FColor::Cyan,
			false,
			0.15f,
			0,
			.2f
		);

		DrawDebugSphere(
			World,
			AimPoint,
			8.f,
			8,
			FColor::Blue,
			false,
			0.15f
		);

		// 실제 상호작용 Sphere Trace
		DrawDebugLine(
			World,
			SphereStart,
			SphereEnd,
			bInteractionHit ? FColor::Green : FColor::Red,
			false,
			0.15f,
			0,
			.2f
		);

		DrawDebugSphere(
			World,
			bInteractionHit ? InteractionHit.Location : SphereEnd,
			SphereRadius,
			12,
			bInteractionHit ? FColor::Green : FColor::Red,
			false,
			0.15f
		);

		if (bInteractionHit)
		{
			DrawDebugString(
				World,
				InteractionHit.ImpactPoint + FVector(0.f, 0.f, 50.f),
				InteractionHit.GetActor()
					? InteractionHit.GetActor()->GetName()
					: TEXT("Unknown"),
				nullptr,
				FColor::Yellow,
				0.15f
			);
		}
	}
#endif
	
	AActor* FoundActor = bInteractionHit ? InteractionHit.GetActor() : nullptr;

	if (FoundActor && FoundActor->Implements<UCPInteractable>())
	{
		CurrentTarget = FoundActor;
		const FText Prompt = ICPInteractable::Execute_GetInteractionPrompt(FoundActor);
		OnPromptChanged.Broadcast(Prompt);
	}
	else
	{
		ClearCurrentTarget();
	}
}

void UCPInteractionComponent::ClearCurrentTarget()
{
	CurrentTarget = nullptr;
	OnPromptChanged.Broadcast(FText::GetEmpty());
}

void UCPInteractionComponent::TryInteract()
{
	if (!CurrentTarget.IsValid())
	{
		return;
	}

	AActor* Target = CurrentTarget.Get();
	if (!Target)
	{
		return;
	}
	
	// 상호작용 가능하다면
	if (ICPInteractable::Execute_CanInteract(Target, GetOwner()))
	{
		// 상호작용 실행
		ICPInteractable::Execute_OnInteract(Target, GetOwner());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't interact with target actor"));
	}
}

