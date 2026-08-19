#include "GameMode/CPLabGameMode.h"

#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "PlayerState/CPLabPlayerState.h"
#include "Quest/QuestManager.h"

ACPLabGameMode::ACPLabGameMode()
{
	GameStateClass = ACPLabGameState::StaticClass();
	PlayerStateClass = ACPLabPlayerState::StaticClass();
	
	SlotActorTag = FName(TEXT("IngredientSlot"));
}

bool ACPLabGameMode::StartPotionRequest(FName QuestId)
{
	if (QuestId.IsNone()) return false;
	
	UQuestManager* QuestManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager || QuestManager->GetQuestState(QuestId) != EQuestState::Accepted) return false;
	
	FCPLabPotionRequest PotionRequest;
	PotionRequest.RequestId = QuestId;
	PotionRequest.DisplayText = QuestManager->GetQuestSummaryText(QuestId);
	
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	return Session->StartRequest(PotionRequest);
}

bool ACPLabGameMode::AdvancePotionRequest()
{
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;
	
	FCPLabPotionRequestState ActiveRequestState;
	if (!Session->GetActiveRequestState(ActiveRequestState)) return false;
	
	if (ActiveRequestState.Phase == ECPLabPotionRequestPhase::Selected){
		return Session->SetRequestPhase(ECPLabPotionRequestPhase::Processing);
	}
	
	if (ActiveRequestState.Phase == ECPLabPotionRequestPhase::Processing){
		return Session->SetRequestPhase(ECPLabPotionRequestPhase::PotionReady);
	}
	
	if (ActiveRequestState.Phase == ECPLabPotionRequestPhase::PotionReady){
		Session->ResetRequest();
		
		PotionDeliveryResult = FCPPotionDeliveryResult{};
		ClearSpawnedIngredients();
		return true;
	}
	return false;
}

bool ACPLabGameMode::RefinePotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform)
{
	TArray<FGameplayTag> SortedEffectTags = EffectTags;
	// 사전 순 정렬
	SortedEffectTags.Sort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return A.ToString() < B.ToString();
	});
	
	return SpawnPotion(SortedEffectTags, SpawnTransform);
	//return AdvancePotionRequest();
}

FCPPotionDeliveryResult ACPLabGameMode::GetPotionDeliveryResult(FName QuestId, const ACPAlchemyProp* PotionProp) const
{
	FCPPotionDeliveryResult Result;
	Result.QuestId = QuestId;
	
	if (!IsValid(PotionProp)) return Result;
	
	const FCPLabIngredientInstance PotionIngredient = PotionProp->GetWorkingIngredient();
	Result.CurrentEffects = PotionIngredient.CurrentEffects;
	
	// TODO: Quest 계산 로직 확정 후 수정
	return Result;
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

bool ACPLabGameMode::SpawnPotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if (!World || !PotionItemData) return false;
	
	UClass* PotionPropClass = PotionItemData->AlchemyPropClass.LoadSynchronous();
	if (!PotionPropClass) return false;
	
	ACPAlchemyProp* PotionProp = World->SpawnActor<ACPAlchemyProp>(PotionPropClass, SpawnTransform);
	if (!IsValid(PotionProp)) return false;
	
	FVector PotionBoundsOrigin, PotionBoundsExtent;
	PotionProp->GetActorBounds(false, PotionBoundsOrigin, PotionBoundsExtent);
	
	// 추가로 보정할 Potion의 Z값 계산 및 보정
	const float PotionBottomZ = PotionBoundsOrigin.Z - PotionBoundsExtent.Z;
	const float SpawnSurfaceZ = SpawnTransform.GetLocation().Z;
	PotionProp->AddActorWorldOffset(FVector(0.f, 0.f, SpawnSurfaceZ - PotionBottomZ));
	
	// 정렬된 Tags로 초기화
	PotionProp->InitializeAlchemyProp(PotionItemData, EffectTags);
	return true;
	//return AdvancePotionRequest();
}
