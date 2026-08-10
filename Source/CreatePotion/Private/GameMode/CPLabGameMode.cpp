#include "GameMode/CPLabGameMode.h"

#include "Data/CPForageableItemData.h"
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

	// PR 이후 제거 예정: 현재 리퀘스트 페이즈 확인용 문자열 변환
	FString GetDebugRequestPhaseText(ECPLabPotionRequestPhase Phase)
	{
		switch (Phase){
		case ECPLabPotionRequestPhase::Queued:
			return TEXT("대기");

		case ECPLabPotionRequestPhase::Preparing:
			return TEXT("재료 준비");

		case ECPLabPotionRequestPhase::Processing:
			return TEXT("가공");

		case ECPLabPotionRequestPhase::PotionReady:
			return TEXT("포션 완성");

		case ECPLabPotionRequestPhase::Delivered:
			return TEXT("납품 완료");

		default:
			return TEXT("알 수 없음");
		}
	}

	// PR 이후 제거 예정: 현재 리퀘스트 페이즈 확인용 DebugMessage
	void ShowDebugRequestPhase(const UCPLabPotionSessionComponent* Session)
	{
		if (!GEngine || !Session) return;

		FCPLabPotionRequestState ActiveRequestState;
		const FString RequestPhaseText = Session->GetActiveRequestState(ActiveRequestState)
			? GetDebugRequestPhaseText(ActiveRequestState.Phase)
			: TEXT("없음");

		GEngine->AddOnScreenDebugMessage(
			-1,
			4.f,
			FColor::Yellow,
			FString::Printf(TEXT("리퀘스트 단계: %s"), *RequestPhaseText));
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
	const bool bAccepted = Session &&
		Session->TrySetRequestPhase(
			GetActiveRequestId(),
			ECPLabPotionRequestPhase::Preparing);

	return bAccepted;
}

bool ACPLabGameMode::TryBeginActiveRequestProcessing()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	const bool bProcessing = Session &&
		Session->TrySetRequestPhase(
			GetActiveRequestId(),
			ECPLabPotionRequestPhase::Processing);

	return bProcessing;
}

bool ACPLabGameMode::TryFinishActivePotion()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	const bool bFinished = Session->TrySetRequestPhase(
		GetActiveRequestId(),
		ECPLabPotionRequestPhase::PotionReady);

	return bFinished;
}

bool ACPLabGameMode::TryDeliverActivePotion()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	const bool bDelivered = Session->TryMarkRequestDelivered(GetActiveRequestId());
	if (!bDelivered) return false;
	
	ResetLabSession();
	return true;
}

bool ACPLabGameMode::PlaceIngredient(int32 SlotIndex, ACPAlchemyProp* Ingredient)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session && Session->PlaceIngredient(SlotIndex, Ingredient);
}

void ACPLabGameMode::DebugAdvanceSessionPhase()
{
	// 월드 Actor가 완성되기 전 세션 진행 상태만 빠르게 확인하는 함수
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return;

	const FCPLabPotionSessionState SessionState = Session->GetSessionState();

	if (SessionState.Phase == ECPLabPotionSessionPhase::WaitingForBell){
		const bool bStarted = TryStartLabSession();
		ShowDebugRequestPhase(Session);
		return;
	}else if (SessionState.Phase == ECPLabPotionSessionPhase::Completed){
		ResetLabSession();
		ShowDebugRequestPhase(Session);
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

	ShowDebugRequestPhase(Session);
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
		TestIngredients.Num(), IngredientSlotActors.Num(), CPLabPotionRequestRules::IngredientSlotCapacity);
	if (SpawnCount <= 0) return false;
	
	bool bPlacedAnyIngredient = false;
	
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	for (int32 SlotIndex = 0; SlotIndex < SpawnCount; ++SlotIndex){
		UCPForageableItemData* ItemData = TestIngredients[SlotIndex];
		AActor* SlotActor = IngredientSlotActors[SlotIndex];
		
		if (!ItemData || !SlotActor) continue;
		
		Session->RegisterIngredientSlotActor(SlotIndex, SlotActor);
		
		UClass* PropClass = ItemData->AlchemyPropClass.LoadSynchronous();
		if (!PropClass) continue;
		
		ACPAlchemyProp* SpawnedProp = World->SpawnActor<ACPAlchemyProp>(PropClass, SlotActor->GetActorTransform());
		if (!SpawnedProp) continue;
		
		// Prop Spawn
		SpawnedProp->InitializeFromItemData(ItemData);
		
		// Slot에는 SpawnedProp 참조를 저장
		if (PlaceIngredient(SlotIndex, SpawnedProp)){
			SpawnedIngredients.Add(SpawnedProp);
			bPlacedAnyIngredient = true;
		}else{
			SpawnedProp->Destroy();
		}
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
