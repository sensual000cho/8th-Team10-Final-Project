#pragma once

#include "CoreMinimal.h"
#include "GameMode/CPGameModeBase.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "Quest/QuestManager.h"
#include "CPLabGameMode.generated.h"

enum class EDeliveryGrade : uint8;
enum class EConditionMatchResult : uint8;
class UCPProcessorComponent;
class ACPAlchemyProp;
class ACPLabGameState;
class UCPLabPotionSessionComponent;
class UCPForageableItemData;
class UCPLabContainerComponent;

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
	TArray<FAlchemyProperty> MinTargetEffects;
	
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Result")
	TArray<FAlchemyProperty> MaxTargetEffects;
	
	UPROPERTY(BlueprintReadOnly, Category = "Lab|Result")
	TArray<FAlchemyProperty> CurrentEffects;
	
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

	// 현재 설정된 테스트 리퀘스트로 세션 시작
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	bool TryStartLabSession();

	// 외부에서 전달받은 리퀘스트 목록으로 세션 시작
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	bool TryStartLabSessionWithRequests(const TArray<FCPLabPotionRequest>& PotionRequests);

	// 진행 중인 포션 세션을 벨 대기 상태로 초기화
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	void ResetLabSession();

	// 여러 리퀘스트 중 플레이어가 진행할 리퀘스트 선택
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TrySelectRequest(FName RequestId);

	// 활성 리퀘스트를 수락하고 재료 준비 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryAcceptActiveRequest();

	// 준비한 재료가 유효하면 가공 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryBeginActiveRequestProcessing();

	// 가공 중인 리퀘스트를 포션 완성 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryFinishActivePotion();
	
	// 포션 Prop을 만들고 현재 결과값을 저장하여 포션 완성 상태로 전환
	bool FinalizePotionAtActor(const AActor* SpawnActor);

	// 완성된 포션을 납품 처리
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryDeliverActivePotion();
	
	// 포션 납품 판정 결과 조회
	UFUNCTION(BlueprintPure, Category = "Lab|Result")
	FCPPotionDeliveryResult GetPotionDeliveryResult() const;
	
	// 다음 리퀘스트 또는 세션 종료 처리
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool ConfirmPotionDeliveryResult();

	// 지정한 슬롯에 재료 배치 요청
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool PlaceIngredient(int32 SlotIndex, ACPAlchemyProp* Ingredient);
	
	// 리퀘스트 종료 시 초기화할 가공기구 등록
	void RegisterProcessor(UCPProcessorComponent* ProcessorComponent);
	
	// Prop을 되돌릴 때 등록된 processor들에게 해당 Prop의 사용 제한 복구 요구
	bool RestoreUseLimit(const ACPAlchemyProp* ItemInstance);
	
	// 재료 생성 단계에서 Spawn 할 재료의 DA 지정
	UFUNCTION(BlueprintCallable, Category = "Lab|Ingredients")
	void SetIngredientsDataAsset(const TArray<UCPForageableItemData*>& IngredientsDataAsset);
	
protected:
	virtual void BeginPlay() override;

private:
	ACPLabGameState* GetLabGameState() const;
	UCPLabPotionSessionComponent* GetPotionSession() const;
	FName GetActiveRequestId() const;
	
	// 퀘스트를 QuestOrder 순서대로 공방 리퀘스트로 변환
	TArray<FCPLabPotionRequest> BuildQuestRequests() const;
	
	// 재료를 Spawn할 Actor 탐색
	void CollectSlotActors(TArray<AActor*>& OutSlotActors) const;
	
	// 재료 Spawn
	bool SpawnIngredients();
	
	// Spawn된 재료 초기화
	void ClearSpawnedIngredients();
	
	// 등록된 가공 기구 상태 초기화
	void ResetProcessors();
	
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
	
	// 초기화할 가공기구를 관리하는 배열
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Processor")
	TArray<TObjectPtr<UCPProcessorComponent>> ProcessorPendings;
	
	// 납품 판정 결과
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Result")
	FCPPotionDeliveryResult PotionDeliveryResult;
};
