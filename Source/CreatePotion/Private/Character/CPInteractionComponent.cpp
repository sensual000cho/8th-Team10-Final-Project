// CPInteractionComponent.cpp


#include "Character/CPInteractionComponent.h"

#include "GameCore/Interface/CPInteractable.h"
#include "GameCore/Interface/CPTimedInteractable.h"
#include "Camera/CameraComponent.h"
#include "GameCore/Interface/CPHighlightable.h"
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Components/MeshComponent.h"

#include "EnhancedInputSubsystems.h"	// IMC 바인딩
#include "EnhancedInputComponent.h"		// IA 바인딩

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

UCPInteractionComponent::UCPInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	InteractionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	InteractionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	InteractionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	InteractionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
}

void UCPInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AActor* OwnerActor = GetOwner())
	{
		Camera = OwnerActor->FindComponentByClass<UCameraComponent>();
	}

	if (Camera)
	{
		// 카메라를 찾았다면 Trace 타이머 시작
		GetWorld()->GetTimerManager().SetTimer(
			TraceTimerHandle,
			this,
			&UCPInteractionComponent::PerformTrace,
			TraceUpdateInterval,
			true
		);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Interaction] 카메라 컴포넌트를 찾을 수 없습니다."));
	}
	
	BindRetryCount = 0;
	GetWorld()->GetTimerManager().SetTimer(
		IMCBindingTimerHandle,
		this,
		&UCPInteractionComponent::TryBindInputMappingContext,
		0.1f,
		true
	);
}

void UCPInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TraceTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(InteractionAlignmentTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(IMCBindingTimerHandle);
	}
	
	Super::EndPlay(EndPlayReason);
}

bool UCPInteractionComponent::IsTimedInteractionAligning() const
{
	return InteractingTarget.IsValid() && bTimedInteractionAligning;
}

FRotator UCPInteractionComponent::GetTimedInteractionFacingRotation() const
{
	return MakeInteractionFacingRotation(InteractingTarget.Get());
}

float UCPInteractionComponent::GetInteractionFacingRotationSpeed() const
{
	return InteractionFacingRotationSpeed;
}

void UCPInteractionComponent::PerformTrace()
{
	if (!IsValid(Camera) || !IsValid(GetOwner()))
    {
        ClearCurrentTarget();
        return;
    }

    UWorld* World = GetWorld();

    if (!IsValid(World) || TraceDistance <= 0.f || InteractionSweepRadius <= 0.f)
    {
        ClearCurrentTarget();
        return;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());

    const FVector CameraStart = Camera->GetComponentLocation();
    const FVector CameraForward = Camera->GetForwardVector().GetSafeNormal();
    const FVector CameraEnd = CameraStart + CameraForward * CameraTraceDistance;

    FHitResult CameraHit;

    const bool bCameraHit = World->LineTraceSingleByChannel(
        CameraHit,
        CameraStart,
        CameraEnd,
        ECC_Visibility,
        QueryParams
    );

    const FVector AimPoint = bCameraHit ? CameraHit.ImpactPoint : CameraEnd;

    const FVector CharacterLocation = GetOwner()->GetActorLocation();
    const FVector SphereStart = CharacterLocation + FVector::UpVector * TraceHeightOffset;
    const FVector DirectionToAim = (AimPoint - SphereStart).GetSafeNormal();

    if (DirectionToAim.IsNearlyZero())
    {
        ClearCurrentTarget();
        return;
    }

    const FVector SphereEnd = SphereStart + DirectionToAim * TraceDistance;

    FCollisionObjectQueryParams ObjectQueryParams;

    for (const TEnumAsByte<EObjectTypeQuery>& ObjectType : InteractionObjectTypes)
    {
        const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(ObjectType.GetValue());

        if (CollisionChannel != ECC_MAX)
        {
            ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);
        }
    }

    TArray<FHitResult> InteractionHits;

    const bool bHasInteractionHits = World->SweepMultiByObjectType(
        InteractionHits,
        SphereStart,
        SphereEnd,
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeSphere(InteractionSweepRadius),
        QueryParams
    );

	ECPInteractionDisplayState BestDisplayState = ECPInteractionDisplayState::Hidden;

	AActor* BestTarget = bHasInteractionHits ? SelectBestInteractionTarget(InteractionHits, CameraStart, CameraForward, BestDisplayState) : nullptr;

#if ENABLE_DRAW_DEBUG
    if (CVarDebugInteraction.GetValueOnGameThread())
    {
        DrawDebugLine(
            World,
            CameraStart,
            AimPoint,
            bCameraHit ? FColor::Blue : FColor::Cyan,
            false,
            TraceUpdateInterval,
            0,
            1.5f
        );

        DrawDebugLine(
            World,
            SphereStart,
            SphereEnd,
            IsValid(BestTarget) ? FColor::Green : FColor::Red,
            false,
            TraceUpdateInterval,
            0,
            1.5f
        );

        DrawDebugSphere(
            World,
            SphereEnd,
            InteractionSweepRadius,
            12,
            IsValid(BestTarget) ? FColor::Green : FColor::Red,
            false,
            TraceUpdateInterval
        );

        TSet<AActor*> DebuggedActors;

        for (const FHitResult& InteractionHit : InteractionHits)
        {
            AActor* HitActor = InteractionHit.GetActor();

            if (!IsValid(HitActor) || DebuggedActors.Contains(HitActor))
            {
                continue;
            }

            DebuggedActors.Add(HitActor);

            if (!HitActor->Implements<UCPInteractable>())
            {
                continue;
            }

            const FVector FocusLocation = GetInteractionFocusLocation(HitActor);
            const FColor CandidateColor = HitActor == BestTarget ? FColor::Green : FColor::Yellow;

            DrawDebugSphere(
                World,
                FocusLocation,
                HitActor == BestTarget ? 14.f : 8.f,
                12,
                CandidateColor,
                false,
                TraceUpdateInterval
            );
        }

        if (IsValid(BestTarget))
        {
            DrawDebugString(
                World,
                GetInteractionFocusLocation(BestTarget) + FVector::UpVector * 30.f,
                BestTarget->GetName(),
                nullptr,
                FColor::Green,
                TraceUpdateInterval
            );
        }
    }
#endif

	UpdateCurrentTarget(BestTarget, BestDisplayState);
}

void UCPInteractionComponent::ClearCurrentTarget()
{
	UpdateCurrentTarget(nullptr, ECPInteractionDisplayState::Hidden);
}

void UCPInteractionComponent::TryInteract()
{
	// 시간형 상호작용 실행중엔 상호작용 Try 하지 않음
	if (InteractingTarget.IsValid())
	{
		return;
	}
	
	if (!CurrentTarget.IsValid())
	{
		return;
	}

	AActor* Target = CurrentTarget.Get();
	AActor* Interactor = GetOwner();
	
	if (!Target || !Target->Implements<UCPInteractable>())
	{
		return;
	}
	
	if (!ICPInteractable::Execute_CanInteract(Target, Interactor))
	{
		return;
	}
	
	if (Target->Implements<UCPTimedInteractable>())
	{
		const float Duration = ICPTimedInteractable::Execute_GetInteractionDuration(Target, Interactor);

		if (Duration > 0.f)
		{
			StartTimedInteraction(Target, Duration);
			return;
		}
	}

	// 시간형 인터페이스가 없으면 일반 상호작용 실행
	ICPInteractable::Execute_OnInteract(Target, Interactor);
	
	RefreshCurrentTargetPresentation();
}

void UCPInteractionComponent::StartTimedInteraction(AActor* Target, float Duration)
{
	if (!IsValid(Target) || Duration <= 0.f || InteractingTarget.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	AActor* Interactor = GetOwner();

	if (!IsValid(World) || !IsValid(Interactor))
	{
		return;
	}

	InteractingTarget = Target;
	InteractionDuration = Duration;
	InteractionElapsedTime = 0.f;
	InteractionStartLocation = Interactor->GetActorLocation();

	bTimedInteractionAligning = true;
	InteractionAlignmentElapsedTime = 0.f;

	// AnimBP를 Release 상태로 변경
	OnInteractionAlignmentStarted.Broadcast();

	OnInteractionProgressChanged.Broadcast(0.f);
	RefreshCurrentTargetPresentation();

	// 이 타이머는 회전하지 않고 유효성만 검사
	World->GetTimerManager().SetTimer(
		InteractionAlignmentTimerHandle,
		this,
		&UCPInteractionComponent::ValidateTimedInteractionAlignment,
		InteractionAlignmentValidationInterval,
		true
	);
}

void UCPInteractionComponent::UpdateTimedInteraction()
{
	if (!InteractingTarget.IsValid())
	{
		CancelTimedInteraction();
		return;
	}

	if (HasInteractorMoved())
	{
		CancelTimedInteraction();
		return;
	}

	InteractionElapsedTime += InteractionUpdateInterval;

	const float Progress = InteractionDuration > 0.f ? FMath::Clamp(InteractionElapsedTime / InteractionDuration, 0.f, 1.f) : 0.f;

	OnInteractionProgressChanged.Broadcast(Progress);

	if (InteractionElapsedTime >= InteractionDuration)
	{
		CompleteTimedInteraction();
	}
}

void UCPInteractionComponent::CompleteTimedInteraction()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(InteractionAlignmentTimerHandle);
	World->GetTimerManager().ClearTimer(InteractionTimerHandle);

	AActor* Target = InteractingTarget.Get();
	AActor* Interactor = GetOwner();

	// 진행상태 초기화
	InteractingTarget.Reset();

	InteractionDuration = 0.f;
	InteractionElapsedTime = 0.f;
	InteractionStartLocation = FVector::ZeroVector;

	bTimedInteractionAligning = false;
	InteractionAlignmentElapsedTime = 0.f;

	if (!IsValid(Target) || !Target->Implements<UCPInteractable>())
	{
		OnInteractionProgressChanged.Broadcast(0.f);
		OnInteractionCancelled.Broadcast();
		RefreshCurrentTargetPresentation();
		return;
	}

	// 시간이 끝나면 기존 OnInteract 호출
	ICPInteractable::Execute_OnInteract(Target, Interactor);
	
	OnInteractionCompleted.Broadcast();
	
	RefreshCurrentTargetPresentation();
}

void UCPInteractionComponent::BeginTimedInteraction()
{
	if (!bTimedInteractionAligning)
	{
		return;
	}

	UWorld* World = GetWorld();
	AActor* Target = InteractingTarget.Get();
	AActor* Interactor = GetOwner();

	if (!IsValid(World) ||
		!IsValid(Target) ||
		!IsValid(Interactor) ||
		!Target->Implements<UCPTimedInteractable>())
	{
		CancelTimedInteraction();
		return;
	}

	World->GetTimerManager().ClearTimer(InteractionAlignmentTimerHandle);

	bTimedInteractionAligning = false;
	InteractionAlignmentElapsedTime = 0.f;

	OnInteractionStarted.Broadcast();
	OnInteractionProgressChanged.Broadcast(0.f);

	// 여기에서 처음으로 조사/채집 몽타주 실행
	ICPTimedInteractable::Execute_OnInteractionStarted(Target, Interactor);

	RefreshCurrentTargetPresentation();

	World->GetTimerManager().SetTimer(
		InteractionTimerHandle,
		this,
		&UCPInteractionComponent::UpdateTimedInteraction,
		InteractionUpdateInterval,
		true
	);
}

void UCPInteractionComponent::SetActorHighlight(AActor* Target, bool bHighlighted)
{
	UE_LOG(LogTemp, Warning, TEXT("[InteractionComponent] SetActorHighlight 호출됨"));
	
	if (!Target) return;
	
	// 하이라이트 인터페이스를 구현한 액터면 그 구현을 우선 사용
	if (Target->Implements<UCPHighlightable>())
	{
		ICPHighlightable::Execute_SetHighlight(Target, bHighlighted);
		return;
	}
	
	if (UMeshComponent* Mesh = Target->FindComponentByClass<UMeshComponent>())
	{
		const TCHAR* Message = HighlightMaterial ? TEXT("Valid") : TEXT("NULL");
		UE_LOG(LogTemp, Warning, TEXT("[InteractionComponent] Found Mesh: %s, HighlightMaterial: %s"),
			*Mesh->GetName(), Message);
		Mesh->SetOverlayMaterial(bHighlighted ? HighlightMaterial : nullptr);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[InteractionComponent] %s 에서 Mesh 발견되지 않음"), *Target->GetName());
	}
}

void UCPInteractionComponent::RefreshCurrentTargetPresentation()
{
	AActor* Target = CurrentTarget.Get();
	AActor* Interactor = GetOwner();

	if (!IsValid(Target) || !IsValid(Interactor) || !Target->Implements<UCPInteractable>())
	{
		ClearCurrentTarget();
		return;
	}

	const ECPInteractionDisplayState NewDisplayState = ResolveInteractionDisplayState(Target, Interactor);

	if (NewDisplayState == ECPInteractionDisplayState::Hidden)
	{
		ClearCurrentTarget();
		return;
	}

	// Enabled -> Disable 등으로 바뀌면 Update.
	if (CurrentDisplayState != NewDisplayState)
	{
		UpdateCurrentTarget(Target, NewDisplayState);
		return;
	}

	// 프롬프트 변경(ex.조사 -> 채집).
	const FText Prompt = ICPInteractable::Execute_GetInteractionPrompt(Target);
	const FName TargetName = ICPInteractable::Execute_GetInteractionName(Target);

	OnPromptChanged.Broadcast(Prompt, TargetName, NewDisplayState);
}

AActor* UCPInteractionComponent::SelectBestInteractionTarget(const TArray<FHitResult>& InteractionHits, const FVector& CameraLocation, const FVector& CameraForward, ECPInteractionDisplayState& OutDisplayState) const
{
    OutDisplayState = ECPInteractionDisplayState::Hidden;

    AActor* OwnerActor = GetOwner();

    if (!IsValid(OwnerActor))
    {
        return nullptr;
    }

    AActor* BestEnabledTarget = nullptr;
    AActor* BestDisabledTarget = nullptr;

    float BestEnabledScore = -BIG_NUMBER;
    float BestDisabledScore = -BIG_NUMBER;

    TSet<AActor*> ProcessedActors;

    for (const FHitResult& InteractionHit : InteractionHits)
    {
        AActor* Candidate = InteractionHit.GetActor();

        if (!IsValid(Candidate) || Candidate == OwnerActor || ProcessedActors.Contains(Candidate))
        {
            continue;
        }

        ProcessedActors.Add(Candidate);

        const ECPInteractionDisplayState CandidateDisplayState = ResolveInteractionDisplayState(Candidate, OwnerActor);

        if (CandidateDisplayState == ECPInteractionDisplayState::Hidden)
        {
            continue;
        }

        const FVector TargetLocation = GetInteractionFocusLocation(Candidate);
        const FVector DirectionToTarget = (TargetLocation - CameraLocation).GetSafeNormal();

        if (DirectionToTarget.IsNearlyZero())
        {
            continue;
        }

        const float CameraAlignment = FVector::DotProduct(CameraForward, DirectionToTarget);

        if (CameraAlignment < MinimumCameraAlignment)
        {
            continue;
        }

        if (!HasLineOfSightToTarget(Candidate, CameraLocation, TargetLocation))
        {
            continue;
        }

        float CandidateScore = CalculateInteractionScore(
            InteractionHit,
            TargetLocation,
            CameraLocation,
            CameraForward
        );

        if (CurrentTarget.Get() == Candidate)
        {
            CandidateScore += CurrentTargetScoreBonus;
        }

        if (CandidateDisplayState == ECPInteractionDisplayState::Enabled)
        {
            if (CandidateScore > BestEnabledScore)
            {
                BestEnabledScore = CandidateScore;
                BestEnabledTarget = Candidate;
            }

            continue;
        }

        if (CandidateDisplayState == ECPInteractionDisplayState::Disabled)
        {
            if (CandidateScore > BestDisabledScore)
            {
                BestDisabledScore = CandidateScore;
                BestDisabledTarget = Candidate;
            }
        }
    }

    // 상호작용 가능한 대상이 항상 Disabled 대상보다 우선.
    if (IsValid(BestEnabledTarget))
    {
        OutDisplayState = ECPInteractionDisplayState::Enabled;
        return BestEnabledTarget;
    }

    if (IsValid(BestDisabledTarget))
    {
        OutDisplayState = ECPInteractionDisplayState::Disabled;
        return BestDisabledTarget;
    }

    return nullptr;
}

FVector UCPInteractionComponent::GetInteractionFocusLocation(const AActor* Target) const
{
	if (!IsValid(Target))
	{
		return FVector::ZeroVector;
	}

	FVector BoundsOrigin;
	FVector BoundsExtent;
	Target->GetActorBounds(true, BoundsOrigin, BoundsExtent);

	if (!BoundsExtent.IsNearlyZero())
	{
		return BoundsOrigin;
	}

	return Target->GetActorLocation();
}

bool UCPInteractionComponent::HasLineOfSightToTarget(const AActor* Target, const FVector& CameraLocation, const FVector& TargetLocation) const
{
	if (!IsValid(Target))
	{
		return false;
	}

	UWorld* World = GetWorld();

	if (!IsValid(World))
	{
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	FHitResult VisibilityHit;

	const bool bHitSomething = World->LineTraceSingleByChannel(
		VisibilityHit,
		CameraLocation,
		TargetLocation,
		ECC_Visibility,
		QueryParams
	);

	if (!bHitSomething)
	{
		return true;
	}

	return VisibilityHit.GetActor() == Target;
}

float UCPInteractionComponent::CalculateInteractionScore(const FHitResult& HitResult, const FVector& TargetLocation, const FVector& CameraLocation, const FVector& CameraForward) const
{
	const FVector DirectionToTarget = (TargetLocation - CameraLocation).GetSafeNormal();
	const float RawAlignment = FVector::DotProduct(CameraForward, DirectionToTarget);

	const float AlignmentScore = FMath::GetMappedRangeValueClamped(
		FVector2D(MinimumCameraAlignment, 1.f),
		FVector2D(0.f, 1.f),
		RawAlignment
	);

	const float SafeTraceDistance = FMath::Max(TraceDistance, 1.f);
	const float DistanceRatio = FMath::Clamp(HitResult.Distance / SafeTraceDistance, 0.f, 1.f);
	const float DistanceScore = 1.f - DistanceRatio;

	return AlignmentScore * AlignmentScoreWeight + DistanceScore * DistanceScoreWeight;
}

void UCPInteractionComponent::UpdateCurrentTarget(AActor* NewTarget, ECPInteractionDisplayState NewDisplayState)
{
	if (!IsValid(NewTarget) || NewDisplayState == ECPInteractionDisplayState::Hidden)
	{
		NewTarget = nullptr;
		NewDisplayState = ECPInteractionDisplayState::Hidden;
	}

	const bool bSameTarget = CurrentTarget.Get() == NewTarget;
	const bool bSameDisplayState = CurrentDisplayState == NewDisplayState;

	if (bSameTarget && bSameDisplayState)
	{
		return;
	}

	if (AActor* PreviousTarget = CurrentTarget.Get())
	{
		SetActorHighlight(PreviousTarget, false);
	}

	CurrentTarget = NewTarget;
	CurrentDisplayState = NewDisplayState;

	if (!IsValid(NewTarget))
	{
		OnPromptChanged.Broadcast(FText::GetEmpty(), NAME_None, ECPInteractionDisplayState::Hidden);
		return;
	}

	// 노란 하이라이트는 실제 상호작용이 가능할 때만 표시
	if (CurrentDisplayState == ECPInteractionDisplayState::Enabled)
	{
		SetActorHighlight(NewTarget, true);
	}

	const FText Prompt = ICPInteractable::Execute_GetInteractionPrompt(NewTarget);
	const FName TargetName = ICPInteractable::Execute_GetInteractionName(NewTarget);

	OnPromptChanged.Broadcast(Prompt, TargetName, CurrentDisplayState);
}

ECPInteractionDisplayState UCPInteractionComponent::ResolveInteractionDisplayState(AActor* Target, AActor* Interactor) const
{
	if (!IsValid(Target) || !IsValid(Interactor))
	{
		return ECPInteractionDisplayState::Hidden;
	}

	if (!Target->Implements<UCPInteractable>())
	{
		return ECPInteractionDisplayState::Hidden;
	}

	if (ICPInteractable::Execute_CanInteract(Target, Interactor))
	{
		return ECPInteractionDisplayState::Enabled;
	}

	if (ICPInteractable::Execute_ShouldShowUnavailableInteraction(Target, Interactor))
	{
		return ECPInteractionDisplayState::Disabled;
	}

	return ECPInteractionDisplayState::Hidden;
}

bool UCPInteractionComponent::HasInteractorMoved() const
{
	const AActor* Interactor = GetOwner();

	if (!IsValid(Interactor))
	{
		return true;
	}

	const float MovedDistanceSquared = FVector::DistSquared(InteractionStartLocation, Interactor->GetActorLocation());

	return MovedDistanceSquared > FMath::Square(InteractionCancelDistance);
}

void UCPInteractionComponent::CancelTimedInteraction()
{
	UWorld* World = GetWorld();

	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(InteractionAlignmentTimerHandle);
		World->GetTimerManager().ClearTimer(InteractionTimerHandle);
	}

	const bool bHadTimedInteraction = InteractingTarget.IsValid() || bTimedInteractionAligning || InteractionDuration > 0.f;

	InteractingTarget.Reset();

	InteractionDuration = 0.f;
	InteractionElapsedTime = 0.f;
	InteractionStartLocation = FVector::ZeroVector;

	bTimedInteractionAligning = false;
	InteractionAlignmentElapsedTime = 0.f;

	if (!bHadTimedInteraction)
	{
		return;
	}

	OnInteractionProgressChanged.Broadcast(0.f);
	OnInteractionCancelled.Broadcast();

	RefreshCurrentTargetPresentation();
}

FRotator UCPInteractionComponent::MakeInteractionFacingRotation(const AActor* Target) const
{
	const AActor* Interactor = GetOwner();

	if (!IsValid(Interactor))
	{
		return FRotator::ZeroRotator;
	}

	if (!IsValid(Target))
	{
		return FRotator(0.f, Interactor->GetActorRotation().Yaw, 0.f);
	}

	const FVector TargetLocation = GetInteractionFocusLocation(Target);
	FVector DirectionToTarget = TargetLocation - Interactor->GetActorLocation();

	DirectionToTarget.Z = 0.f;

	if (DirectionToTarget.IsNearlyZero())
	{
		return FRotator(0.f, Interactor->GetActorRotation().Yaw, 0.f);
	}

	return FRotator(0.f, DirectionToTarget.Rotation().Yaw, 0.f);
}

void UCPInteractionComponent::ValidateTimedInteractionAlignment()
{
	if (!bTimedInteractionAligning)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(InteractionAlignmentTimerHandle);
		}

		return;
	}

	if (!IsValid(GetOwner()) || !InteractingTarget.IsValid())
	{
		CancelTimedInteraction();
		return;
	}

	if (HasInteractorMoved())
	{
		CancelTimedInteraction();
		return;
	}

	InteractionAlignmentElapsedTime += InteractionAlignmentValidationInterval;
	
	if (InteractionAlignmentElapsedTime >= InteractionFacingReleaseDuration)
	{
		if (FinishTimedInteractionAlignment())
		{
			return;
		}
	}

	if (InteractionAlignmentElapsedTime >= InteractionAlignmentTimeout)
	{
		CancelTimedInteraction();
	}
}

bool UCPInteractionComponent::FinishTimedInteractionAlignment()
{
	if (!bTimedInteractionAligning)
	{
		return false;
	}

	AActor* Interactor = GetOwner();
	AActor* Target = InteractingTarget.Get();

	if (!IsValid(Interactor) || !IsValid(Target))
	{
		CancelTimedInteraction();
		return false;
	}

	if (HasInteractorMoved())
	{
		CancelTimedInteraction();
		return false;
	}

	const FRotator TargetRotation = MakeInteractionFacingRotation(Target);
	const float CurrentYaw = Interactor->GetActorRotation().Yaw;

	const float RemainingYaw = FMath::Abs(
		FMath::FindDeltaAngleDegrees(
			CurrentYaw,
			TargetRotation.Yaw
		)
	);

	if (RemainingYaw > InteractionFacingTolerance)
	{
		return false;
	}

	// 마지막 미세 오차 정리
	Interactor->SetActorRotation(TargetRotation);

	BeginTimedInteraction();
	return true;
}

#pragma region IMC
void UCPInteractionComponent::TryBindInputMappingContext()
{
	if (!InteractionMappingContext || !InteractAction)
	{
		// IMC, IA가 할당 안 되어 있으면 시도할 필요조차 없으니 타이머 즉시 종료
		GetWorld()->GetTimerManager().ClearTimer(IMCBindingTimerHandle);
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* MyPawn = Cast<APawn>(GetOwner());

	if (PC && PC->InputComponent)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// 플레이어 인터렉션용 IMC를 추가, priority = 1, 값이 높을 수록 우선
			Subsystem->AddMappingContext(InteractionMappingContext, 1);
			UE_LOG(LogTemp, Warning, TEXT("[Interaction] IMC 바인딩 성공"));
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(MyPawn->InputComponent))
		{
			// Started(누른 순간)에 TryInteract 함수 실행
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &UCPInteractionComponent::TryInteract);
			GetWorld()->GetTimerManager().ClearTimer(IMCBindingTimerHandle);	// 바인딩 성공시 타이머 즉시 종료
			UE_LOG(LogTemp, Warning, TEXT("[Interaction] IA 바인딩 성공"));
		}
	}
	else
	{
		// 실패 시 시도한 횟수 카운트 증가
		BindRetryCount++;

		// 50회(약 5초)가 지나도 안 되면 포기
		if (BindRetryCount >= 50)
		{
			GetWorld()->GetTimerManager().ClearTimer(IMCBindingTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("[Interaction] 입력 바인딩 실패"));
		}
	}
}
#pragma endregion
