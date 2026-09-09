#include "GameMode/CPLabGameMode.h"

#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPPotionActor.h"
#include "PlayerState/CPLabPlayerState.h"

ACPLabGameMode::ACPLabGameMode() : bActiveRequest(false)
{
	GameStateClass = ACPLabGameState::StaticClass();
	PlayerStateClass = ACPLabPlayerState::StaticClass();
}

bool ACPLabGameMode::StartPotionRequest()
{
	if (bActiveRequest) return false;
	
	bActiveRequest = true;
	return true;
}

bool ACPLabGameMode::ClearPotionRequest()
{
	if (!bActiveRequest) return false;
	
	bActiveRequest = false;
	return true;
}

bool ACPLabGameMode::IsActiveRequest() const
{
	return bActiveRequest;
}

bool ACPLabGameMode::CreatePotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform,
                                  const FVector& SpawnImpulse)
{
	TArray<FGameplayTag> SortedEffectTags = EffectTags;
	// 사전 순 정렬
	SortedEffectTags.Sort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return A.ToString() < B.ToString();
	});
	
	if (!SpawnPotion(SortedEffectTags, SpawnTransform, SpawnImpulse)) return false;
	
	return true;
}

bool ACPLabGameMode::SpawnPotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform, const FVector& SpawnImpulse)
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
	// Impuse 적용
	Potion->ApplySpawnImpulse(SpawnImpulse);
	return true;
}
