#pragma once

#include "CoreMinimal.h"
#include "GameMode/CPGameModeBase.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "Quest/QuestManager.h"
#include "CPLabGameMode.generated.h"

class ACPPotionActor;
enum class EDeliveryGrade : uint8;
class ACPAlchemyProp;
class ACPLabGameState;
class UCPForageableItemData;

USTRUCT(BlueprintType)
struct FCPPotionDeliveryResult
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Result")
	FName QuestId = NAME_None;
	
	// 최종 납품 등급
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Result")
	EDeliveryGrade DeliveryGrade = EDeliveryGrade::Fail;
	
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Result")
	TArray<FGameplayTag> CurrentEffects;
	
	// 기본 보상
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Result")
	int32 RewardAmount = 100;
	
	// 보너스, 팁을 합산
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Result")
	int32 TipAmount = 0;
};

// Blueprint와 월드 Actor가 포션 세션 기능을 호출하는 입구
UCLASS()
class CREATEPOTION_API ACPLabGameMode : public ACPGameModeBase
{
	GENERATED_BODY()

public:
	ACPLabGameMode();

	// 선택한 QuestId로 리퀘스트를 시작하고, 제조 상태로 진입
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool StartPotionRequest(FName QuestId);
	
	// 현재 리퀘스트를 다음 단계로 진행
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool AdvancePotionRequest();
	
	// Tags 사전 순 정렬, (TODO)태그 조합 반영
	UFUNCTION(BlueprintCallable, Category = "Lab|Potion")
	bool RefinePotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform, const FVector& SpawnImpulse);
	
	// 상호작용으로 전달된 Potion Prop의 결과를 납품 결과 구조로 변환
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	FCPPotionDeliveryResult GetPotionDeliveryResult(FName QuestId, const ACPPotionActor* PotionActor);
	
	UFUNCTION(BlueprintPure, Category = "Lab|Request")
	bool HasActiveRequest() const;
	
	FName GetActiveRequestId() const;
	
private:	
	// Spawn된 재료 초기화
	void ClearSpawnedIngredients();
	
	// Potion Spawn
	bool SpawnPotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform, const FVector& SpawnImpulse);

private:	
	// 초기 재료 배치에 사용할 DA
	UPROPERTY(EditDefaultsOnly, Category = "Lab|Ingredients")
	TArray<TObjectPtr<UCPForageableItemData>> Ingredients;
	
	// 포션 Prop을 만들 때 사용할 DA
	UPROPERTY(EditDefaultsOnly, Category = "Lab|Potion")
	TObjectPtr<UCPForageableItemData> PotionItemData;
		
	// 재료를 놓을 SlotActor 탐색용 태그
	UPROPERTY(EditDefaultsOnly, Category = "Lab|Session")
	FName SlotActorTag;
	
	// 생성된 재료를 관리하는 배열
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Ingredients")
	TArray<TObjectPtr<ACPAlchemyProp>> SpawnedIngredients;
	
	// 현재 공방 요청 QuestId
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Request")
	FName ActiveRequestId = NAME_None;
	
	// 납품 판정 결과
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Result")
	FCPPotionDeliveryResult PotionDeliveryResult;
};
