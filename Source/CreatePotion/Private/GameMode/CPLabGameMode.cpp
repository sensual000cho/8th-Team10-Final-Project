#include "GameMode/CPLabGameMode.h"

#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "Lab/Component/CPProcessorComponent.h"
#include "PlayerState/CPLabPlayerState.h"
#include "Quest/QuestManager.h"

#include "EngineUtils.h"		// Iterator
#include "Lab/Actor/CPLabContainerActor.h"

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
	// 관련 사항 구현 후 삭제(프로토타입 퀘스트를 Accepted 상태로 변경)
	AcceptProtoTypeQuest();
	return TryStartLabSessionWithRequests(BuildQuestRequests());
}

bool ACPLabGameMode::TryStartLabSessionWithRequests(const TArray<FCPLabPotionRequest>& PotionRequests)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	return Session->StartSession(PotionRequests);
}

void ACPLabGameMode::ResetLabSession()
{
	ClearSpawnedIngredients();
	ResetProcessors();
	
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

bool ACPLabGameMode::FinalizePotionAtActor(const AActor* SpawnActor)
{
	UWorld* World = GetWorld();
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!World || !Session || !PotionItemData || !IsValid(SpawnActor)) return false;
	
	UClass* PotionPropClass = PotionItemData->AlchemyPropClass.LoadSynchronous();
	if (!PotionPropClass) return false;
	
	ACPAlchemyProp* PotionProp = World->SpawnActor<ACPAlchemyProp>(PotionPropClass, SpawnActor->GetActorTransform());
	if (!PotionProp) return false;
	
	// 기구, Potion의 경계 계산
	FVector SpawnActorOrigin, SpawnActorExtent;
	FVector PotionOrigin, PotionExtent;
	SpawnActor->GetActorBounds(true, SpawnActorOrigin, SpawnActorExtent);
	PotionProp->GetActorBounds(true, PotionOrigin, PotionExtent);
	
	// 보정할 Z 값 계산
	const float SpawnActorTopZ = SpawnActorOrigin.Z + SpawnActorExtent.Z;
	const float PotionBottomZ = PotionOrigin.Z - PotionExtent.Z;
	const float ZOffset = SpawnActorTopZ - PotionBottomZ;
	
	// Z 값 보정
	PotionProp->AddActorWorldOffset(FVector(0.f, 0.f, ZOffset), false);
	
	if (!Session->FinalizePotionResult(PotionProp, PotionItemData)){
		PotionProp->Destroy();
		return false;
	}
	
	if (!TryFinishActivePotion()){
		PotionProp->Destroy();
		return false;
	}
	
	SpawnedIngredients.Add(PotionProp);
	return true;
}


bool ACPLabGameMode::TryDeliverActivePotion()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	FCPLabPotionRequestState ActiveRequestState;
	if (!Session->GetActiveRequestState(ActiveRequestState) || 
		ActiveRequestState.Phase != ECPLabPotionRequestPhase::PotionReady) return false;
	
	UQuestManager* QuestManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return false;
	
	// 퀘스트 Id, 완성한 포션 Tag, 목표 Tag
	const FName QuestId = ActiveRequestState.PotionRequest.RequestId;
	const TArray<FAlchemyProperty>& PotionResult = Session->GetPotionResult();
	const TArray<FQuestEffectRequirement> TargetRequirements = QuestManager->GetQuestEffectRequirements(QuestId);
	
	// 현재 납품 결과로 다시 구성한다
	PotionDeliveryResult = FCPPotionDeliveryResult{};
	PotionDeliveryResult.QuestId = QuestId;
	PotionDeliveryResult.DeliveryGrade = QuestManager->TryDeliver(QuestId, PotionResult);
	PotionDeliveryResult.CurrentEffects = PotionResult;
	
	if (PotionDeliveryResult.DeliveryGrade == EDeliveryGrade::Fail){
		PotionDeliveryResult.RewardAmount = 0;
	}else if (PotionDeliveryResult.DeliveryGrade == EDeliveryGrade::Good || 
		PotionDeliveryResult.DeliveryGrade == EDeliveryGrade::Okay){
		PotionDeliveryResult.RewardAmount *= 0.5f;
	}
	
	// 최소/최대 목표를 저장
	for (const FQuestEffectRequirement& Requirement : TargetRequirements){
		FAlchemyProperty MinTargetEffect;
		MinTargetEffect.Tag = Requirement.Axis;
		MinTargetEffect.Value = Requirement.MinValue;
		PotionDeliveryResult.MinTargetEffects.Add(MinTargetEffect);
		
		FAlchemyProperty MaxTargetEffect;
		MaxTargetEffect.Tag = Requirement.Axis;
		MaxTargetEffect.Value = Requirement.MaxValue;
		PotionDeliveryResult.MaxTargetEffects.Add(MaxTargetEffect);
	}
	
	return true;
}

FCPPotionDeliveryResult ACPLabGameMode::GetPotionDeliveryResult() const
{
	return PotionDeliveryResult;
}

bool ACPLabGameMode::ConfirmPotionDeliveryResult()
{
	// QuestId가 저장되었을 때에만 진행
	if (PotionDeliveryResult.QuestId.IsNone()) return false;
	
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	// Phase 전환(Delivered)
	const bool bDelivered = Session->TryMarkRequestDelivered(PotionDeliveryResult.QuestId);
	if (!bDelivered) return false;
	
	// PotionDeliveryResult 초기화
	PotionDeliveryResult = FCPPotionDeliveryResult{};
	
	// 모든 Request가 끝났으면 세션 종료
	if (Session->GetSessionState().Phase == ECPLabPotionSessionPhase::Completed){
		ResetLabSession();
		return true;
	}
	
	// Request가 남아있으면 재료와 가공 기구만 정리
	ClearSpawnedIngredients();
	ResetProcessors();
	return true;
}

bool ACPLabGameMode::PlaceIngredient(int32 SlotIndex, ACPAlchemyProp* Ingredient)
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	return Session && Session->PlaceIngredient(SlotIndex, Ingredient);
}

void ACPLabGameMode::RegisterProcessor(UCPProcessorComponent* ProcessorComponent)
{
	if (!IsValid(ProcessorComponent) || ProcessorPendings.Contains(ProcessorComponent)) return;
	
	ProcessorPendings.Add(ProcessorComponent);
}

bool ACPLabGameMode::RestoreUseLimit(const ACPAlchemyProp* ItemInstance)
{
	if (!IsValid(ItemInstance)) return false;
	
	bool bForgotAny = false;
	for (UCPProcessorComponent* Processor : ProcessorPendings){
		if (!IsValid(Processor)) continue;
		// Processor가 이 Prop 떄문에 소모된 사용 제한을 복구한다
		if (Processor->RestoreUseLimit(ItemInstance)){
			bForgotAny = true;
		}
	}
	return bForgotAny;
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

void ACPLabGameMode::SetIngredientsDataAsset(const TArray<UCPForageableItemData*>& IngredientsDataAsset)
{
	if (IngredientsDataAsset.Num() <= 0) return;
		TryBeginActiveRequestProcessing();
		return;
	}
	
	Ingredients.Reset();
	Ingredients.Reserve(IngredientsDataAsset.Num());
	
	for (UCPForageableItemData* IngredientItemData : IngredientsDataAsset){
		Ingredients.Add(IngredientItemData);
	}
	
	// 배치 성공하면 제조 Phase로 전환
	if (SpawnIngredients()){
		TryBeginActiveRequestProcessing();
	}
}

void ACPLabGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ACPLabContainerActor> It(GetWorld()); It; ++It)
	{
		if (ACPLabContainerActor* LabActor = *It)
		{
			// 찾은 액터의 컴포넌트를 캐싱
			CachedLabContainer = LabActor->LabContainerComponent;
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] Lab Container Cached"));
			break;
		}
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

void ACPLabGameMode::AcceptProtoTypeQuest() const
{
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;
	
	QuestManager->AcceptQuest(FName(TEXT("Origin_Q001")));
}

TArray<FCPLabPotionRequest> ACPLabGameMode::BuildQuestRequests() const
{
	TArray<FCPLabPotionRequest> PotionRequests;
	
	const UGameInstance* GameInstance = GetGameInstance();
	const UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	
	if (!QuestManager) return PotionRequests;
	
	const TArray<FName> TrackedQuestIds = QuestManager->GetAllTrackedQuestIDs();
	PotionRequests.Reserve(FMath::Min(TrackedQuestIds.Num(), CPLabPotionRequestRules::MaxRequestCount));
	
	for (const FName& QuestId : TrackedQuestIds){
		if (PotionRequests.Num() >= CPLabPotionRequestRules::MaxRequestCount) break;
		if (QuestId.IsNone() || QuestManager->GetQuestState(QuestId) != EQuestState::Accepted) continue;
		
		FCPLabPotionRequest PotionRequest;
		PotionRequest.RequestId = QuestId;
		PotionRequest.DisplayText = QuestManager->GetQuestSummaryText(QuestId);
		
		PotionRequests.Add(PotionRequest);
	}
	
	return PotionRequests;
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
		Ingredients.Num(), IngredientSlotActors.Num(), CPLabPotionRequestRules::IngredientSlotCapacity);
	if (SpawnCount <= 0) return false;
	
	bool bPlacedAnyIngredient = false;
	
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	for (int32 SlotIndex = 0; SlotIndex < SpawnCount; ++SlotIndex){
		UCPForageableItemData* ItemData = Ingredients[SlotIndex];
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

void ACPLabGameMode::ResetProcessors()
{
	for (UCPProcessorComponent* Processor : ProcessorPendings){
		if (!IsValid(Processor)) continue;
		Processor->ResetProcessor();
	}
	
	ProcessorPendings.Reset();
}
