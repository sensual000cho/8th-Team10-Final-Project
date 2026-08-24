// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CPTrajectoryPreviewComponent.h"

#include "Character/CPCarryComponent.h"
#include "Character/GAS/Abilities/GA_CPThrowProp.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/SplineMeshComponent.h"

UCPTrajectoryPreviewComponent::UCPTrajectoryPreviewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCastShadow(false);
	SetHiddenInGame(true);
	SetVisibility(false);
	
	ThrowAbilityClass = UGA_CPThrowProp::StaticClass();
}

void UCPTrajectoryPreviewComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	CarryComponent = OwnerActor->FindComponentByClass<UCPCarryComponent>();
	if (!IsValid(CarryComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("[TrajectoryPreview] %s에 CarryComponent가 없습니다."), *GetNameSafe(OwnerActor));
		return;
	}

	CreatePreviewSegments();

	CarryComponent->OnHeldPropChanged.AddUniqueDynamic(this, &UCPTrajectoryPreviewComponent::HandleHeldPropChanged);

	// BeginPlay 전에 이미 무언가를 들고 있는 경우까지 초기 동기화
	HandleHeldPropChanged(CarryComponent->GetHeldProp());
}

void UCPTrajectoryPreviewComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(CarryComponent))
	{
		CarryComponent->OnHeldPropChanged.RemoveDynamic(this, &UCPTrajectoryPreviewComponent::HandleHeldPropChanged);
	}

	DeactivatePreview();
	CarryComponent = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UCPTrajectoryPreviewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!IsValid(HeldProp))
	{
		DeactivatePreview();
		return;
	}

	UpdatePreview();
}

void UCPTrajectoryPreviewComponent::HandleHeldPropChanged(ACPThrowablePropBase* NewHeldProp)
{
	if (IsValid(NewHeldProp))
	{
		ActivatePreview(NewHeldProp);
		return;
	}

	DeactivatePreview();
}

void UCPTrajectoryPreviewComponent::ActivatePreview(ACPThrowablePropBase* InHeldProp)
{
	if (!IsValid(InHeldProp))
	{
		DeactivatePreview();
		return;
	}

	HeldProp = InHeldProp;
	CachedPredictionRadius = ResolvePredictionRadius(HeldProp);
	SetHiddenInGame(false);
	SetVisibility(true);
	SetComponentTickEnabled(true);
}

void UCPTrajectoryPreviewComponent::DeactivatePreview()
{
	SetComponentTickEnabled(false);
	HidePreviewSegments();
	SetHiddenInGame(true);
	SetVisibility(false);
	ClearSplinePoints();
	CachedPredictionRadius = DefaultPredictionRadius;
	HeldProp = nullptr;
}

float UCPTrajectoryPreviewComponent::ResolvePredictionRadius(const ACPThrowablePropBase* InHeldProp) const
{
	if (!IsValid(InHeldProp))
	{
		return DefaultPredictionRadius;
	}

	const UPrimitiveComponent* CollisionComponent =
		Cast<UPrimitiveComponent>(InHeldProp->GetRootComponent());

	if (!IsValid(CollisionComponent))
	{
		return DefaultPredictionRadius;
	}

	const float ShortestExtent = CollisionComponent->Bounds.BoxExtent.GetMin();
	if (!FMath::IsFinite(ShortestExtent) || ShortestExtent <= KINDA_SMALL_NUMBER)
	{
		return DefaultPredictionRadius;
	}

	return ShortestExtent;
}

bool UCPTrajectoryPreviewComponent::CalculateTrajectory(FPredictProjectilePathResult& OutPathResult) const
{
	if (!IsValid(HeldProp) || !ThrowAbilityClass)
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn))
	{
		return false;
	}

	const UGA_CPThrowProp* ThrowAbility = ThrowAbilityClass.GetDefaultObject();
	if (!IsValid(ThrowAbility))
	{
		return false;
	}

	const float ThrowSpeed = ThrowAbility->GetThrowSpeed();
	if (ThrowSpeed <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const APlayerController* PlayerController =
		Cast<APlayerController>(OwnerPawn->GetController());

	const FVector ThrowDirection =
		ThrowAbility->CalculateThrowDirection(OwnerPawn, PlayerController);

	if (ThrowDirection.IsNearlyZero())
	{
		return false;
	}

	// 최대 표시 길이를 계산하기에 충분한 엔진 내부 시뮬레이션 범위.
	// 사용자에게 노출되는 궤적 길이 설정과는 별개의 값이다.
	const float InternalPredictionDuration =
		FMath::Max((MaxPredictionLength / ThrowSpeed) * 2.f, 0.5f);

	FPredictProjectilePathParams PredictionParams(
		CachedPredictionRadius, 
		HeldProp->GetActorLocation(), 
		ThrowDirection * ThrowSpeed,
		InternalPredictionDuration, 
		ECC_Visibility, 
		GetOwner());
	PredictionParams.ActorsToIgnore.Add(HeldProp);

	UGameplayStatics::PredictProjectilePath(this, PredictionParams, OutPathResult);

	return OutPathResult.PathData.Num() >= 2;
}

void UCPTrajectoryPreviewComponent::UpdatePreview()
{
	// 실제 투척과 같은 초기 방향과 속도로 전체 예측 경로를 생성.
	FPredictProjectilePathResult PathResult;
	if (!CalculateTrajectory(PathResult))
	{
		ClearSplinePoints();
		HidePreviewSegments();
		return;
	}

	const TArray<FPredictProjectilePathPointData>& PathPoints = PathResult.PathData;

	float TotalPathLength = 0.f;
	float ApexPathLength = 0.f;
	float HighestPointZ = PathPoints[0].Location.Z;

	// 경로를 따라 거리를 누적하며 전체 길이와 정점까지의 길이를 계산.
	for (int32 PointIndex = 1; PointIndex < PathPoints.Num(); ++PointIndex)
	{
		const FVector& PreviousLocation = PathPoints[PointIndex - 1].Location;
		const FVector& CurrentLocation = PathPoints[PointIndex].Location;

		TotalPathLength += FVector::Distance(PreviousLocation, CurrentLocation);

		if (CurrentLocation.Z > HighestPointZ)
		{
			HighestPointZ = CurrentLocation.Z;
			ApexPathLength = TotalPathLength;
		}
	}

	if (TotalPathLength <= KINDA_SMALL_NUMBER)
	{
		ClearSplinePoints();
		HidePreviewSegments();
		return;
	}

	// 정점 이후 구간을 포함하되 설정한 최소·최대 표시 길이 안으로 제한.
	const float DesiredPathLength = ApexPathLength + PostApexPredictionLength;
	const float VisiblePathLength = FMath::Min(FMath::Clamp(DesiredPathLength, MinPredictionLength, MaxPredictionLength), TotalPathLength);

	// 이전 프레임의 점을 비우고 이번 프레임의 표시 구간을 다시 구성.
	ClearSplinePoints(false);
	AddSplinePoint(PathPoints[0].Location, ESplineCoordinateSpace::World, false);

	float CurrentPathLength = 0.f;

	for (int32 PointIndex = 1; PointIndex < PathPoints.Num(); ++PointIndex)
	{
		const FVector& PreviousLocation = PathPoints[PointIndex - 1].Location;
		const FVector& CurrentLocation = PathPoints[PointIndex].Location;
		const float SegmentLength = FVector::Distance(PreviousLocation, CurrentLocation);

		if (SegmentLength <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		// 목표 길이를 넘어가는 마지막 구간은 정확한 끝 위치를 보간.
		if (CurrentPathLength + SegmentLength >= VisiblePathLength)
		{
			const float RemainingLength = VisiblePathLength - CurrentPathLength;
			const float SegmentAlpha = RemainingLength / SegmentLength;
			const FVector FinalLocation = FMath::Lerp(PreviousLocation, CurrentLocation, SegmentAlpha);

			AddSplinePoint(FinalLocation, ESplineCoordinateSpace::World, false);
			break;
		}

		AddSplinePoint(CurrentLocation, ESplineCoordinateSpace::World, false);
		CurrentPathLength += SegmentLength;
	}

	UpdateSpline();
	UpdatePreviewSegments();
}

void UCPTrajectoryPreviewComponent::CreatePreviewSegments()
{
	if (!PreviewSegments.IsEmpty() || !IsValid(PreviewSegmentMesh))
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	PreviewSegments.Reserve(MaxPreviewSegments);

	for (int32 SegmentIndex = 0; SegmentIndex < MaxPreviewSegments; ++SegmentIndex)
	{
		const FName SegmentName(*FString::Printf(TEXT("TrajectoryPreviewSegment_%02d"), SegmentIndex));
		USplineMeshComponent* PreviewSegment = NewObject<USplineMeshComponent>(OwnerActor, SegmentName);

		if (!IsValid(PreviewSegment))
		{
			continue;
		}

		OwnerActor->AddInstanceComponent(PreviewSegment);
		PreviewSegment->SetupAttachment(this);
		PreviewSegment->SetMobility(EComponentMobility::Movable);
		PreviewSegment->SetStaticMesh(PreviewSegmentMesh);
		PreviewSegment->SetForwardAxis(ESplineMeshAxis::X, false);
		PreviewSegment->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PreviewSegment->SetGenerateOverlapEvents(false);
		PreviewSegment->SetCanEverAffectNavigation(false);
		PreviewSegment->SetCastShadow(false);

		if (PreviewMaterial)
		{
			PreviewSegment->SetMaterial(0, PreviewMaterial);
		}

		PreviewSegment->SetHiddenInGame(true);
		PreviewSegment->SetVisibility(false);
		PreviewSegment->RegisterComponent();

		PreviewSegments.Add(PreviewSegment);
	}
}

void UCPTrajectoryPreviewComponent::UpdatePreviewSegments()
{
	const int32 VisibleSegmentCount = FMath::Min(FMath::Max(GetNumberOfSplinePoints() - 1, 0), PreviewSegments.Num());
	const FVector2D LineScale(PreviewLineScale, PreviewLineScale);

	for (int32 SegmentIndex = 0; SegmentIndex < VisibleSegmentCount; ++SegmentIndex)
	{
		USplineMeshComponent* PreviewSegment = PreviewSegments[SegmentIndex];
		if (!IsValid(PreviewSegment))
		{
			continue;
		}

		const FVector StartLocation = GetLocationAtSplinePoint(SegmentIndex, ESplineCoordinateSpace::Local);
		const FVector StartTangent = GetTangentAtSplinePoint(SegmentIndex, ESplineCoordinateSpace::Local);
		const FVector EndLocation = GetLocationAtSplinePoint(SegmentIndex + 1, ESplineCoordinateSpace::Local);
		const FVector EndTangent = GetTangentAtSplinePoint(SegmentIndex + 1, ESplineCoordinateSpace::Local);

		PreviewSegment->SetStartScale(LineScale, false);
		PreviewSegment->SetEndScale(LineScale, false);
		PreviewSegment->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, true);
		PreviewSegment->SetHiddenInGame(false);
		PreviewSegment->SetVisibility(true);
	}

	for (int32 SegmentIndex = VisibleSegmentCount; SegmentIndex < PreviewSegments.Num(); ++SegmentIndex)
	{
		USplineMeshComponent* PreviewSegment = PreviewSegments[SegmentIndex];
		if (!IsValid(PreviewSegment))
		{
			continue;
		}

		PreviewSegment->SetHiddenInGame(true);
		PreviewSegment->SetVisibility(false);
	}
}

void UCPTrajectoryPreviewComponent::HidePreviewSegments()
{
	for (USplineMeshComponent* PreviewSegment : PreviewSegments)
	{
		if (!IsValid(PreviewSegment))
		{
			continue;
		}

		PreviewSegment->SetHiddenInGame(true);
		PreviewSegment->SetVisibility(false);
	}
}
