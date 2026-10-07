#include "Quest/Components/CPQuestLocationComponent.h"
#include "Quest/QuestManager.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UCPQuestLocationComponent::UCPQuestLocationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UCPQuestLocationComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// BeginPlay 이전 시점 → MoveComponent가 저장된 위치로 옮기기 전의 레벨 배치 위치
	if (const AActor* Owner = GetOwner())
	{
		HomeTransform = Owner->GetActorTransform();
	}
}

void UCPQuestLocationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UQuestManager* QuestManager = GetQuestManager())
	{
		QuestManager->OnQuestUpdated.AddDynamic(this, &UCPQuestLocationComponent::HandleQuestUpdated);
	}

	// 레벨 진입 시 현재 퀘스트 상태 기준으로 위치 복원 (페이드 없이 즉시)
	ApplyLocation(ResolveDestination());
}

void UCPQuestLocationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UQuestManager* QuestManager = GetQuestManager())
	{
		QuestManager->OnQuestUpdated.RemoveDynamic(this, &UCPQuestLocationComponent::HandleQuestUpdated);
	}

	// 페이드 도중 레벨이 끝나면 화면이 어두운 채로 남지 않도록 복구
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FadeTimerHandle);
	}
	if (bFading)
	{
		if (APlayerCameraManager* CameraManager = GetCameraManager())
		{
			CameraManager->StopCameraFade();
			CameraManager->StartCameraFade(1.f, 0.f, 0.01f, FLinearColor::Black);
		}
		bFading = false;
	}

	Super::EndPlay(EndPlayReason);
}

void UCPQuestLocationComponent::HandleQuestUpdated(FName QuestID, EQuestState NewState)
{
	if (bRefreshLocked)
	{
		bPendingRefresh = true;
		return;
	}
	RefreshLocation();
}

void UCPQuestLocationComponent::SetRefreshLocked(bool bLocked)
{
	bRefreshLocked = bLocked;

	if (!bLocked && bPendingRefresh)
	{
		bPendingRefresh = false;
		RefreshLocation();
	}
}

AActor* UCPQuestLocationComponent::ResolveDestination() const
{
	const UQuestManager* QuestManager = GetQuestManager();
	if (!QuestManager) return nullptr;

	// 조건이 맞는 규칙 중 마지막 것을 적용
	AActor* Target = nullptr;
	for (const FCPQuestLocationRule& Rule : LocationRules)
	{
		if (Rule.Destination && QuestManager->GetQuestState(Rule.QuestID) == Rule.TriggerState)
		{
			Target = Rule.Destination;
		}
	}
	return Target;
}

void UCPQuestLocationComponent::RefreshLocation()
{
	// 페이드 중에는 끝난 뒤에 다시 계산
	if (bFading)
	{
		bRefreshAfterFade = true;
		return;
	}

	AActor* Target = ResolveDestination();

	// 이미 그 위치에 있으면 무시
	if (Target == CurrentDestination) return;

	APlayerCameraManager* CameraManager = bUseFade ? GetCameraManager() : nullptr;
	if (!CameraManager)
	{
		ApplyLocation(Target);
		return;
	}

	// 페이드 아웃 → (어두워지면) 이동 → 페이드 인
	bFading = true;
	PendingDestination = Target;
	CameraManager->StartCameraFade(0.f, 1.f, FadeDuration, FLinearColor::Black, false, true);

	GetWorld()->GetTimerManager().SetTimer(
		FadeTimerHandle, this, &UCPQuestLocationComponent::OnFadeOutFinished, FadeDuration, false);
}

void UCPQuestLocationComponent::OnFadeOutFinished()
{
	ApplyLocation(PendingDestination);
	PendingDestination = nullptr;

	if (APlayerCameraManager* CameraManager = GetCameraManager())
	{
		CameraManager->StartCameraFade(1.f, 0.f, FadeDuration, FLinearColor::Black, false, false);
	}

	GetWorld()->GetTimerManager().SetTimer(
		FadeTimerHandle, this, &UCPQuestLocationComponent::OnFadeInFinished, FadeDuration, false);
}

void UCPQuestLocationComponent::OnFadeInFinished()
{
	bFading = false;

	// 페이드 도중 들어온 요청이 있으면 지금 상태 기준으로 다시 계산
	if (bRefreshAfterFade)
	{
		bRefreshAfterFade = false;
		RefreshLocation();

		// 새 페이드가 시작됐으면 그쪽이 끝날 때 알림
		if (bFading) return;
	}

	OnFadeFinished.Broadcast();
}

void UCPQuestLocationComponent::ApplyLocation(AActor* Target)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 이미 그 위치에 있으면 무시
	if (Target == CurrentDestination && bFading == false && Target != nullptr)
	{
		return;
	}
	CurrentDestination = Target;

	FVector NewLocation = HomeTransform.GetLocation();
	FQuat NewRotation = HomeTransform.GetRotation();

	if (Target)
	{
		NewLocation = Target->GetActorLocation();
		NewRotation = Target->GetActorQuat();

		// Character는 캡슐 중심이 액터 위치라, 바닥에 파묻히지 않게 절반 높이만큼 올림
		if (const ACharacter* Character = Cast<ACharacter>(Owner))
		{
			NewLocation.Z += Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
	}

	Owner->SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
	OnLocationChanged.Broadcast(Target != nullptr);
}

UQuestManager* UCPQuestLocationComponent::GetQuestManager() const
{
	const UWorld* World = GetWorld();
	const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
}

APlayerCameraManager* UCPQuestLocationComponent::GetCameraManager() const
{
	return UGameplayStatics::GetPlayerCameraManager(this, 0);
}