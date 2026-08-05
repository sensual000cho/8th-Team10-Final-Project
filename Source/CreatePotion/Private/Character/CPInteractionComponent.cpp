// CPInteractionComponent.cpp


#include "Character/CPInteractionComponent.h"
#include "GameCore/Interface/CPInteractable.h"

void UCPInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	
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
	FHitResult Hit;
	FVector Start = GetOwner()->GetActorLocation();
	FVector End = GetOwner()->GetActorForwardVector() * TraceDistance;
	
	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(50.f)
	);
	
	AActor* Found = bHit? Hit.GetActor() : nullptr;
	
	// CPInteractable를 가지고 있는지 확인
	if (Found && Found->Implements<UCPInteractable>())
	{
		CurrentTarget = Found;
		FText Prompt = ICPInteractable::Execute_GetInteractionPrompt(Found);
		// UI에 프롬프트 표시하기 (F: 채집하기 / F: 물약 만들기 등...), UI에서 바인딩 필요
		OnPromptChanged.Broadcast(Prompt);
	}
	else
	{
		CurrentTarget = nullptr;
		OnPromptChanged.Broadcast(FText::GetEmpty());
	}
}

void UCPInteractionComponent::TryInteract()
{
	if (!CurrentTarget.IsValid())
	{
		return;
	}
	
	AActor* Target = CurrentTarget.Get();
	
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

