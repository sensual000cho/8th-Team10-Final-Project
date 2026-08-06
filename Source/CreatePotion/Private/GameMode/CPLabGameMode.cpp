#include "GameMode/CPLabGameMode.h"

#include "Data/CPForageableItemData.h"
#include "Engine/Engine.h"
#include "GameState/CPLabGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "PlayerState/CPLabPlayerState.h"

namespace
{
	// 실제 리퀘스트 시스템이 연결되기 전 사용할 간단한 테스트 데이터 생성
	FCPLabPotionRequest MakeTestRequest(
		FName RequestId,
		FText DisplayText)
	{
		FCPLabPotionRequest PotionRequest;
		PotionRequest.RequestId = RequestId;
		PotionRequest.DisplayText = MoveTemp(DisplayText);
		return PotionRequest;
	}
}

ACPLabGameMode::ACPLabGameMode()
{
	GameStateClass = ACPLabGameState::StaticClass();
	PlayerStateClass = ACPLabPlayerState::StaticClass();

	// 현재 프로토타입은 리퀘스트 하나로 전체 제조 흐름 확인
	DefaultTestRequests =
	{
		MakeTestRequest(
			FName(TEXT("TestPotionRequest01")),
			FText::FromString(TEXT("Create the first test potion.")))
	};
	
	SlotActorTag = FName(TEXT("IngredientSlot"));
}

bool ACPLabGameMode::TryStartLabSession()
{
	return TryStartLabSessionWithRequests(DefaultTestRequests);
}

bool ACPLabGameMode::TryStartLabSessionWithRequests(const TArray<FCPLabPotionRequest>& PotionRequests)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	const bool bStarted = Session->StartSession(PotionRequests);
	if (bStarted){
		if (GEngine){
			FCPLabPotionRequestState ActiveRequestState;
			if (!Session->GetActiveRequestState(ActiveRequestState)){
				GEngine->AddOnScreenDebugMessage(
					-1, 3.0f, FColor::Red, TEXT("[Lab] 활성화된 리퀘스트가 없습니다."));
				return bStarted;
			}
			GEngine->AddOnScreenDebugMessage(
				-1, 3.0f, FColor::Cyan, 
				FString::Printf(TEXT("[Lab] %s"), *ActiveRequestState.PotionRequest.DisplayText.ToString()));
		}
		SpawnIngredients();
	}
	return bStarted;
}

void ACPLabGameMode::ResetLabSession()
{
	ClearSpawnedIngredients();
	
	if (UCPLabPotionSessionComponent* Session = GetPotionSession()){
		Session->ResetSession();
	}
}

bool ACPLabGameMode::TrySelectRequest(FName RequestId)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session && Session->TrySetActiveRequest(RequestId);
}

bool ACPLabGameMode::TryAcceptActiveRequest()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TrySetRequestPhase(
			GetActiveRequestId(),
			ECPLabPotionRequestPhase::Preparing);
}

bool ACPLabGameMode::TryBeginActiveRequestProcessing()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TrySetRequestPhase(
			GetActiveRequestId(),
			ECPLabPotionRequestPhase::Processing);
}

bool ACPLabGameMode::TryFinishActivePotion()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TrySetRequestPhase(
			GetActiveRequestId(),
			ECPLabPotionRequestPhase::PotionReady);
}

bool ACPLabGameMode::TryDeliverActivePotion()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TryMarkRequestDelivered(GetActiveRequestId());
}

bool ACPLabGameMode::TryPlaceIngredient(
	FName RequestId, int32 SlotIndex, const FCPLabIngredientInstance& Ingredient)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TryPlaceIngredient(
			RequestId,
			SlotIndex,
			Ingredient);
}

bool ACPLabGameMode::TryClearIngredient(FName RequestId, int32 SlotIndex)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session &&
		Session->TryClearIngredient(RequestId, SlotIndex);
}

bool ACPLabGameMode::TryCreateWorkingIngredient(
	FName RequestId, int32 SlotIndex, FCPLabIngredientInstance& OutWorkingIngredient) const
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session && Session->TryCreateWorkingCopy(
		RequestId,
		SlotIndex,
		OutWorkingIngredient);
}

bool ACPLabGameMode::TryCommitWorkingIngredient(
	FName RequestId, int32 SlotIndex, const FCPLabIngredientInstance& WorkingIngredient)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session && Session->TryCommitWorkingCopy(
		RequestId,
		SlotIndex,
		WorkingIngredient);
}

void ACPLabGameMode::DebugAdvanceSessionPhase()
{
	// 월드 Actor가 완성되기 전 세션 진행 상태만 빠르게 확인하는 함수
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return;

	const FCPLabPotionSessionState SessionState = Session->GetSessionState();

	if (SessionState.Phase == ECPLabPotionSessionPhase::WaitingForBell){
		const bool bStarted = TryStartLabSession();
		return;
	}else if (SessionState.Phase == ECPLabPotionSessionPhase::Completed){
		ResetLabSession();
		return;
	}

	FCPLabPotionRequestState ActiveRequestState;
	if (!Session->GetActiveRequestState(ActiveRequestState)) return;

	switch (ActiveRequestState.Phase){
	case ECPLabPotionRequestPhase::Queued:
		TryAcceptActiveRequest();
		break;

	case ECPLabPotionRequestPhase::Preparing:
		TryBeginActiveRequestProcessing();
		break;

	case ECPLabPotionRequestPhase::Processing:
		TryFinishActivePotion();
		break;

	case ECPLabPotionRequestPhase::PotionReady:
		TryDeliverActivePotion();
		break;

	default:
		break;
	}
}

ACPLabGameState* ACPLabGameMode::GetLabGameState() const
{
	return Cast<ACPLabGameState>(GameState);
}

UCPLabPotionSessionComponent* ACPLabGameMode::GetPotionSession() const
{
	const ACPLabGameState* LabState = GetLabGameState();
	return LabState ? LabState->GetPotionSession() : nullptr;
}

FName ACPLabGameMode::GetActiveRequestId() const
{
	const UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) {
		return NAME_None;
	}

	FCPLabPotionRequestState ActiveRequestState;
	return Session->GetActiveRequestState(ActiveRequestState)
		? ActiveRequestState.PotionRequest.RequestId
		: NAME_None;
}

void ACPLabGameMode::CollectSlotActors(TArray<AActor*>& OutSlotActors) const
{
	OutSlotActors.Reset();
	if (SlotActorTag.IsNone()) return;
	
	// SlotActorTag을 가진 Actor을 찾아 정렬
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), SlotActorTag, OutSlotActors);
	OutSlotActors.Sort([](const AActor& A, const AActor& B)
	{
		return A.GetActorNameOrLabel() < B.GetActorNameOrLabel();
	});
}

bool ACPLabGameMode::SpawnIngredients()
{
	UWorld* World = GetWorld();
	if (!World) return false;
	
	// RequestPhase을 재료 준비 단계로 변경
	const FName ActiveRequestId = GetActiveRequestId();
	if (ActiveRequestId.IsNone() || !TryAcceptActiveRequest()) return false;
	
	// Tag를 기준으로 재료를 생성할 Actor 탐색
	TArray<AActor*> IngredientSlotActors;
	CollectSlotActors(IngredientSlotActors);
	
	const int32 SpawnCount = FMath::Min3(
		TestIngredients.Num(),
		IngredientSlotActors.Num(), 
		CPLabPotionRequestRules::IngredientSlotCapacity);
	if (SpawnCount <= 0) return false;
	
	bool bPlacedAnyIngredient = false;
	int32 PlacedCount = 0;
	
	for (int32 SlotIndex = 0; SlotIndex < SpawnCount; ++SlotIndex){
		UCPForageableItemData* ItemData = TestIngredients[SlotIndex];
		AActor* SlotActor = IngredientSlotActors[SlotIndex];
		
		if (!ItemData || !SlotActor) continue;
		
		UClass* PropClass = ItemData->AlchemyPropClass.LoadSynchronous();
		if (!PropClass) continue;
		
		ACPAlchemyProp* SpawnedProp = World->SpawnActor<ACPAlchemyProp>(
			PropClass, SlotActor->GetActorTransform());
		if (!SpawnedProp) continue;
		
		// Prop Spawn
		SpawnedProp->InitializeFromItemData(ItemData);
		
		// Slot 액터와 재료 Actor의 실제 Bounds 가져옴
		FVector SlotOrigin, SlotExtent;
		FVector PropOrigin, PropExtent;
		SlotActor->GetActorBounds(false, SlotOrigin, SlotExtent);
		SpawnedProp->GetActorBounds(false, PropOrigin, PropExtent);
		
		// 재료가 파묻치지 않기 위한 z값 계산
		const float SlotSurfaceZ = SlotOrigin.Z + SlotExtent.Z;
		const float PropBottomZ = PropOrigin.Z - PropExtent.Z;
		const float ZOffset = SlotSurfaceZ - PropBottomZ;
		
		// 높이 보정
		SpawnedProp->AddActorWorldOffset(FVector(0.f, 0.f, ZOffset), false);
		
		// 생성된 Prop의 재료 데이터를 현재 Slot에 저장
		// 변경 예정 사양: Slot에는 SpawnedProp 참조를 저장
		const FCPLabIngredientInstance WorkingIngredient = SpawnedProp->GetWorkingIngredient();
		if (TryPlaceIngredient(ActiveRequestId, SlotIndex, WorkingIngredient)){
			SpawnedIngredients.Add(SpawnedProp);
			bPlacedAnyIngredient = true;
			++PlacedCount;
		}else{
			SpawnedProp->Destroy();
		}
	}
	
	if (GEngine){
		GEngine->AddOnScreenDebugMessage(
			-1, 3.f, bPlacedAnyIngredient ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("[Lab] %d개 재료가 생성 후 배치되었습니다."), PlacedCount));
	}
	
	return bPlacedAnyIngredient;
}

void ACPLabGameMode::ClearSpawnedIngredients()
{
	// 남아있는 재료들 정리
	for (ACPAlchemyProp* Ingredient : SpawnedIngredients){
		if (IsValid(Ingredient)){
			Ingredient->Destroy();
		}
	}
	SpawnedIngredients.Reset();
}
