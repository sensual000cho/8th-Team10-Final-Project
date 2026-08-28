#include "GameMode/CPLabGameMode.h"

#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Actor/CPPotionActor.h"
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
	
	if (HasActiveRequest()) return false;

	ActiveRequestId = QuestId;
	PotionDeliveryResult = FCPPotionDeliveryResult{};
	return true;
}

bool ACPLabGameMode::AdvancePotionRequest()
{
	if (!HasActiveRequest()) return false;

	ActiveRequestId = NAME_None;
	PotionDeliveryResult = FCPPotionDeliveryResult{};
	ClearSpawnedIngredients();
	return true;
}

bool ACPLabGameMode::RefinePotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform)
{
	TArray<FGameplayTag> SortedEffectTags = EffectTags;
	// 사전 순 정렬
	SortedEffectTags.Sort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return A.ToString() < B.ToString();
	});
	
	if (!SpawnPotion(SortedEffectTags, SpawnTransform)) return false;
	
	return true;
}

FCPPotionDeliveryResult ACPLabGameMode::GetPotionDeliveryResult(FName QuestId, const ACPPotionActor* PotionActor)
{
	if (PotionDeliveryResult.QuestId == QuestId) return PotionDeliveryResult;
	
	PotionDeliveryResult = FCPPotionDeliveryResult{};
	PotionDeliveryResult.QuestId = QuestId;
	
	if (QuestId.IsNone() || !IsValid(PotionActor)) return PotionDeliveryResult;
	
	PotionDeliveryResult.CurrentEffects = PotionActor->GetWorkingIngredient().CurrentEffects;
	
	UQuestManager* QuestManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return PotionDeliveryResult;
	
	PotionDeliveryResult.DeliveryGrade = QuestManager->TryDeliver(QuestId, PotionDeliveryResult.CurrentEffects);
	PotionDeliveryResult.RewardAmount = QuestManager->GetRewardGold(QuestId);
	PotionDeliveryResult.TipAmount = 0;
	
	return PotionDeliveryResult;
}

bool ACPLabGameMode::HasActiveRequest() const
{
	return !ActiveRequestId.IsNone();
}

FName ACPLabGameMode::GetActiveRequestId() const
{
	return ActiveRequestId;
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
	
	UClass* PotionActorClass = PotionItemData->AlchemyPropClass.LoadSynchronous();
	if (!PotionActorClass) return false;
	
	ACPPotionActor* Potion = World->SpawnActor<ACPPotionActor>(PotionActorClass, SpawnTransform);
	if (!IsValid(Potion)) return false;
	
	FVector PotionBoundsOrigin, PotionBoundsExtent;
	Potion->GetActorBounds(false, PotionBoundsOrigin, PotionBoundsExtent);
	
	// 추가로 보정할 Potion의 Z값 계산 및 보정
	const float PotionBottomZ = PotionBoundsOrigin.Z - PotionBoundsExtent.Z;
	const float SpawnSurfaceZ = SpawnTransform.GetLocation().Z;
	Potion->AddActorWorldOffset(FVector(0.f, 0.f, SpawnSurfaceZ - PotionBottomZ));
	
	// 정렬된 Tags로 초기화
	Potion->InitializeFromItemData(PotionItemData, EffectTags);
	return true;
	//return AdvancePotionRequest();
}
