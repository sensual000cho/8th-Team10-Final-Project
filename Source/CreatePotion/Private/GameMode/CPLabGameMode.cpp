#include "GameMode/CPLabGameMode.h"

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

void ACPLabGameMode::ResetPotionRequest()
{
	ClearSpawnedIngredients();
	
	if (UCPLabPotionSessionComponent* Session = GetPotionSession()){
		Session->ResetRequest();
	}
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
