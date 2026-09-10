#include "NPC/CPNPCMoveComponent.h"
#include "NPC/CPBaseNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

DEFINE_LOG_CATEGORY(LogCPNPC);

UCPNPCMoveComponent::UCPNPCMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCPNPCMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
	{
		MovementComp->bUseRVOAvoidance = true;
		MovementComp->AvoidanceConsiderationRadius = AvoidanceRadius;
		MovementComp->AvoidanceWeight = AvoidanceWeight;
	}

	AIController = Cast<AAIController>(OwnerCharacter->GetController());
	if (AIController)
	{
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UCPNPCMoveComponent::OnMoveCompleted);
		AIController->ReceiveMoveCompleted.AddDynamic(this, &UCPNPCMoveComponent::OnMoveCompleted);
	}
	
	InitializeMovementData();

	const FName StorageKey = GetMovementStorageKey();
	if (!StorageKey.IsNone())
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			if (UCPNPCSubsystem* Subsystem = GI->GetSubsystem<UCPNPCSubsystem>())
			{
				FCPNPCMovementSaveData SaveData;
				if (Subsystem->GetNPCMovementData(StorageKey, SaveData))
				{
					RestoreMovementState(SaveData);
				}
			}
		}
	}

	StartMoving();
}

void UCPNPCMoveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SaveMovementState();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MoveWaitTimerHandle);
	}

	if (AIController)
	{
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UCPNPCMoveComponent::OnMoveCompleted);
	}

	Super::EndPlay(EndPlayReason);
}

void UCPNPCMoveComponent::StartMoving()
{
	bIsMovePaused = false;
	if (TargetPoints.Num() == 0)
	{
		return;
	}

	MoveToCurrentIndex();
}

void UCPNPCMoveComponent::StopMoving()
{
	bIsMoving = false;
	bIsMovePaused = true;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MoveWaitTimerHandle);
	}

	if (AIController)
	{
		AIController->StopMovement();
	}
}

void UCPNPCMoveComponent::ApplySpeed(float Speed)
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = Speed;
			//UE_LOG(LogCPNPC, Log, TEXT("[%s] 이동 속도 적용: %f"), *GetNameSafe(GetOwner()), MoveComp->MaxWalkSpeed);
		}
	}
}

void UCPNPCMoveComponent::SetRVOAvoidanceEnabled(bool bEnable)
{
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MovementComp = OwnerChar->GetCharacterMovement())
		{
			MovementComp->bUseRVOAvoidance = bEnable;
		}
	}
}

void UCPNPCMoveComponent::RestoreMovementState(const FCPNPCMovementSaveData& SaveData)
{
	if (!SaveData.bHasSavedTransform)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	FVector TargetLocation = SaveData.SavedLocation;

	if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
	{
		MoveComp->UpdateComponentVelocity();
		MoveComp->FindFloor(OwnerCharacter->GetActorLocation(), MoveComp->CurrentFloor, false);
	}

	OwnerCharacter->SetActorLocationAndRotation(
		TargetLocation,
		SaveData.SavedRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	CurrentPointIndex = SaveData.NextPointIndex;

	/*UE_LOG(LogCPNPC, Log, TEXT("[NPC 위치 복원] 액터: [%s] | 복원된 좌표: %s | 이어서 갈 인덱스: %d"),
		*GetNameSafe(OwnerCharacter),
		*SaveData.SavedLocation.ToCompactString(),
		CurrentPointIndex);*/
}

void UCPNPCMoveComponent::SaveMovementState()
{
	const FName StorageKey = GetMovementStorageKey();
	if (StorageKey.IsNone())
	{
		return;
	}

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UCPNPCSubsystem* Subsystem = GI->GetSubsystem<UCPNPCSubsystem>())
		{
			FCPNPCMovementSaveData SaveData;
			if (AActor* Owner = GetOwner())
			{
				SaveData.SavedLocation = Owner->GetActorLocation();
				SaveData.SavedRotation = Owner->GetActorRotation();
			}
			SaveData.NextPointIndex = CurrentPointIndex;
			SaveData.bHasSavedTransform = true;
			Subsystem->SaveNPCMovementData(StorageKey, SaveData);

			//UE_LOG(LogCPNPC, Log, TEXT("[NPC 위치 저장] 키: [%s] | 좌표: %s | 다음 목표 인덱스: %d"),*StorageKey.ToString(),*SaveData.SavedLocation.ToCompactString(),SaveData.NextPointIndex);
		}
	}
}

void UCPNPCMoveComponent::InitializeMovementData()
{
	ACPBaseNPC* BaseNPC = Cast<ACPBaseNPC>(GetOwner());
	if (!BaseNPC || !BaseNPC->GetNPCData())
	{
		return;
	}

	const UCPNPCDataAsset* DA = BaseNPC->GetNPCData();

	TargetPoints.Empty();
	CurrentPointIndex = 0;

	if (const FCPNPCTargetRow* Row = DA->TargetRowHandle.GetRow<FCPNPCTargetRow>(TEXT("InitializeMovementData")))
	{
		const FString CurrentLevelName = GetCurrentLevelIdentifier();
		const FCPNPCRoute* SelectedRoute = nullptr;

		for (const FCPNPCRoute& Route : Row->Routes)
		{
			if (Route.TargetLevel.IsValid() || !Route.TargetLevel.IsNull())
			{
				FString TargetMapName = Route.TargetLevel.GetAssetName();
				if (TargetMapName.Equals(CurrentLevelName, ESearchCase::IgnoreCase))
				{
					SelectedRoute = &Route;
					break;
				}
			}
		}

		if (SelectedRoute)
		{
			TargetPoints = SelectedRoute->TargetPoints;
			bLoop = SelectedRoute->bLoop;
			CurrentPointIndex = 0;
		}
		else
		{
			//UE_LOG(LogCPNPC, Log, TEXT("[%s] 현재 레벨 [%s]에 지정된 루트가 없어 정지 상태를 유지합니다."),*GetNameSafe(GetOwner()), *CurrentLevelName);
		}
	}

	ApplySpeed(DA->MoveSpeed);
}

void UCPNPCMoveComponent::MoveToCurrentIndex()
{
	//UE_LOG(LogCPNPC, Warning, TEXT("[%s] MoveToCurrentIndex 호출됨! 인덱스: %d"),*GetNameSafe(GetOwner()), CurrentPointIndex);

	if (!AIController || TargetPoints.Num() == 0)
	{
		return;
	}

	if (!TargetPoints.IsValidIndex(CurrentPointIndex))
	{
		CurrentPointIndex = 0;
	}

	FVector Destination = TargetPoints[CurrentPointIndex].Point;

	// 네비메시 바닥 스냅 (Z축 보정)
	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation ProjectedLocation;
		const FVector QueryExtent(100.f, 100.f, 500.f);

		if (NavSys->ProjectPointToNavigation(Destination, ProjectedLocation, QueryExtent))
		{
			Destination = ProjectedLocation.Location;
		}
	}

	bIsMoving = true;

	/*if (AActor* OwnerActor = GetOwner())
	{
		const FVector CurrentLoc = OwnerActor->GetActorLocation();
		const float Dist3D = FVector::Dist(CurrentLoc, Destination);
		const float Dist2D = FVector::Dist2D(CurrentLoc, Destination);

		UE_LOG(LogCPNPC, Error, TEXT("[%s] [이동 시도] 목표: %s | 현재위치: %s | 3D거리: %.1f | 2D거리: %.1f | AcceptanceRadius: %.1f"),
			*GetNameSafe(OwnerActor),
			*Destination.ToString(),
			*CurrentLoc.ToString(),
			Dist3D,
			Dist2D,
			AcceptanceRadius);
	}*/

	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		Destination,
		AcceptanceRadius,
		true,  // bStopOnOverlap
		true,  // bUsePathfinding
		false, // 수동 투영 완료 상태
		true   // bCanStrafe
	);

	//UE_LOG(LogCPNPC, Warning, TEXT("[%s] MoveToLocation 결과: %s"),*GetNameSafe(GetOwner()),*UEnum::GetValueAsString(MoveResult));

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		bIsMoving = false;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				MoveWaitTimerHandle,
				this,
				&UCPNPCMoveComponent::MoveToCurrentIndex,
				1.0f,
				false
			);
		}
	}
	else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		OnMoveCompleted(FAIRequestID::InvalidRequest, EPathFollowingResult::Success);
	}
}

void UCPNPCMoveComponent::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	bIsMoving = false;

	if (bIsMovePaused)
	{
		return;
	}

	FVector ArrivedLocation = FVector::ZeroVector;
	float WaitTime = 0.0f;

	if (TargetPoints.IsValidIndex(CurrentPointIndex))
	{
		WaitTime = TargetPoints[CurrentPointIndex].WaitSeconds;
		ArrivedLocation = TargetPoints[CurrentPointIndex].Point;
	}
	//UE_LOG(LogCPNPC, Log, TEXT("[%s] 도착 좌표: %s | 대기 시간: %.2f | 현재 인덱스: %d"),*GetNameSafe(GetOwner()), *ArrivedLocation.ToString(), WaitTime, CurrentPointIndex);

	if (Result == EPathFollowingResult::Success)
	{
		CurrentPointIndex++;
		if (CurrentPointIndex >= TargetPoints.Num())
		{
			if (bLoop)
			{
				CurrentPointIndex = 0;
			}
			else
			{
				return;
			}
		}

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				MoveWaitTimerHandle,
				this,
				&UCPNPCMoveComponent::MoveToCurrentIndex,
				FMath::Max(0.1f, WaitTime),
				false
			);
		}
	}
	else if (Result == EPathFollowingResult::Blocked || Result == EPathFollowingResult::Aborted)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				MoveWaitTimerHandle,
				this,
				&UCPNPCMoveComponent::MoveToCurrentIndex,
				1.0f,
				false
			);
		}
	}
}

FString UCPNPCMoveComponent::GetCurrentLevelIdentifier() const
{
	UWorld* World = GetWorld();
	if (!World) return FString();

	FString RawLevelName;

	if (AActor* Owner = GetOwner())
	{
		if (ULevel* OwnerLevel = Owner->GetLevel())
		{
			RawLevelName = FPackageName::GetShortName(OwnerLevel->GetPackage());
		}
	}

	if (RawLevelName.IsEmpty())
	{
		RawLevelName = FPackageName::GetShortName(World->GetPackage());
	}

	return UWorld::RemovePIEPrefix(RawLevelName);
}
FName UCPNPCMoveComponent::GetMovementStorageKey() const
{
	ACPBaseNPC* BaseNPC = Cast<ACPBaseNPC>(GetOwner());
	if (!BaseNPC || BaseNPC->GetPotionNPCId().IsNone())
	{
		return NAME_None;
	}

	FString LevelName = GetCurrentLevelIdentifier();
	return FName(*FString::Printf(TEXT("%s_%s"), *BaseNPC->GetPotionNPCId().ToString(), *LevelName));
}