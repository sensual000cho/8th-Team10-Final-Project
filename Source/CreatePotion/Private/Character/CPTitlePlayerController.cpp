// Fill out your copyright notice in the Description page of Project Settings.


#include "CPTitlePlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DelayAction.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BookMenuComponent.h"
#include "UI/Widgets/Title/CPBookMenuActor.h"

void ACPTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// 태그로 레벨에 배치된 카메라 액터 캐싱
	PreTitleCamera = FindActorByTag(PreTitleCameraTag);
	TitleCamera = FindActorByTag(TitleCameraTag);
	
	
	// BookMenu 액터 캐싱
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			APawn* PlayerPawn = GetPawn();
			if (!IsValid(PlayerPawn)) return;
			UBookMenuComponent* BookMenuComp = PlayerPawn->FindComponentByClass<UBookMenuComponent>();
			if (!IsValid(BookMenuComp)) return;
			BookMenuActor =  Cast<ACPBookMenuActor>(BookMenuComp->BookMenuActor);
			if (!IsValid(BookMenuActor))
			{
				UE_LOG(LogTemp, Warning, TEXT("BookMenuActor가 유효하지 않습니다."));
			}		
		}));
	}
	
}

void ACPTitlePlayerController::StartTitleBlend()
{
	if (bBlendStarted || !TitleCamera) return;
	bBlendStarted = true;
	
	BlendHandle = PlayerCameraManager->OnBlendComplete().AddUObject(this, &ACPTitlePlayerController::HandleCameraBlendComplete);

	SetViewTargetWithBlend(TitleCamera, BlendTime, VTBlend_EaseInOut, 2.f);
	
	// 이벤트가 오지 않는 경우 대비
	GetWorldTimerManager().SetTimer(FallbackTimer, this, &ACPTitlePlayerController::HandleCameraBlendComplete, BlendTime + 0.1f, false);
}

void ACPTitlePlayerController::HandleCameraBlendComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("카메라 블렌드 종료"));
	if (!bMenuLocked) return;
	bMenuLocked = false;
	
	GetWorldTimerManager().ClearTimer(FallbackTimer);
	if (IsValid(PlayerCameraManager))
	{
		PlayerCameraManager->OnBlendComplete().Remove(BlendHandle);
	}
	
}

AActor* ACPTitlePlayerController::FindActorByTag(FName Tag) const
{
	
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsWithTag(this, TitleCameraTag, Found);
	
	if (Found.Num() != 1)
	{
		return nullptr;
	}
	return Found[0];
}