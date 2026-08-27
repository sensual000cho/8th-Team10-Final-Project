// CPInteractionComponent.cpp


#include "Character/CPInteractionComponent.h"

#include "GameCore/Interface/CPInteractable.h"
#include "GameCore/Interface/CPTimedInteractable.h"
#include "Camera/CameraComponent.h"
#include "GameCore/Interface/CPHighlightable.h"
#include "HAL/IConsoleManager.h"

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
			0.15f,
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
		GetWorld()->GetTimerManager().ClearTimer(
			TraceTimerHandle
		);

		GetWorld()->GetTimerManager().ClearTimer(
			InteractionTimerHandle
		);

		GetWorld()->GetTimerManager().ClearTimer(
			IMCBindingTimerHandle
		);
	}
	
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
		// 같은 대상을 계속 보고 있다면 매번 호출 X
		if (CurrentTarget.Get() == FoundActor)
		{
			return;
		}
		
		// 이전 대상 하이라이트 끄기
		if (AActor* PrevTarget = CurrentTarget.Get())
		{
			SetActorHighlight(PrevTarget, false);
		}
		
		CurrentTarget = FoundActor;
		
		// 새 대상 하이라이트 켜기
		SetActorHighlight(FoundActor, true);
		
		const FText Prompt = ICPInteractable::Execute_GetInteractionPrompt(FoundActor);
		const FName TargetName = ICPInteractable::Execute_GetInteractionName(FoundActor);
		OnPromptChanged.Broadcast(Prompt, TargetName);
	}
	else
	{
		ClearCurrentTarget();
	}
}

void UCPInteractionComponent::ClearCurrentTarget()
{
	if (CurrentTarget.IsExplicitlyNull())
	{
		return; // 이미 비어있는 상태면 중복 브로드캐스트 방지

	}

	if (AActor* Target = CurrentTarget.Get())
	{
		SetActorHighlight(Target, false);
	}

	CurrentTarget.Reset();
	OnPromptChanged.Broadcast(FText::GetEmpty(), NAME_None);
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
}

void UCPInteractionComponent::StartTimedInteraction(AActor* Target, float Duration)
{
	if (!Target || Duration <= 0.f || InteractingTarget.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	InteractingTarget = Target;
	InteractionDuration = Duration;
	InteractionElapsedTime = 0.f;

	ICPTimedInteractable::Execute_OnInteractionStarted(Target, GetOwner());
	
	OnInteractionStarted.Broadcast();

	World->GetTimerManager().SetTimer(
		InteractionTimerHandle,
		this,
		&UCPInteractionComponent::UpdateTimedInteraction,
		InteractionUpdateInterval,
		true
	);
}

void UCPInteractionComponent::UpdateTimedInteraction()
{
	if (!InteractingTarget.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
		return;
	}

	InteractionElapsedTime += InteractionUpdateInterval;
	
	OnInteractionProgressChanged.Broadcast(InteractionElapsedTime / InteractionDuration);

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

	World->GetTimerManager().ClearTimer(InteractionTimerHandle);

	AActor* Target = InteractingTarget.Get();
	AActor* Interactor = GetOwner();

	// 진행상태 초기화
	InteractingTarget.Reset();

	InteractionDuration = 0.f;
	InteractionElapsedTime = 0.f;
	
	OnInteractionCompleted.Broadcast();

	if (!Target || !Target->Implements<UCPInteractable>())
	{
		return;
	}

	// 시간이 끝나면 기존 OnInteract 호출
	ICPInteractable::Execute_OnInteract(
		Target,
		Interactor
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
