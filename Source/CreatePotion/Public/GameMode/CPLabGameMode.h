#pragma once

#include "CoreMinimal.h"
#include "GameMode/CPGameModeBase.h"
#include "Quest/QuestManager.h"
#include "CPLabGameMode.generated.h"

class ACPPotionActor;
enum class EDeliveryGrade : uint8;
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
	
	// 리퀘스트를 초기화
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool ClearPotionRequest();
	
	// Tags 사전 순 정렬, (TODO)태그 조합 반영
	UFUNCTION(BlueprintCallable, Category = "Lab|Potion")
	bool CreatePotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform, const FVector& SpawnImpulse);
	
	// 포션 효과 태그로 납품 결과를 생성하고, 이미 생성된 결과가 있으면 저장된 결과를 반환
	FCPPotionDeliveryResult GetPotionDeliveryResult(const TArray<FGameplayTag>& PotionEffectTags = TArray<FGameplayTag>());
	
	FName GetActiveRequestId() const;
	
private:	
	// Potion Spawn
	bool SpawnPotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform, const FVector& SpawnImpulse);

private:
	// 포션 Prop을 만들 때 사용할 DA
	UPROPERTY(EditDefaultsOnly, Category = "Lab|Potion")
	TObjectPtr<UCPForageableItemData> PotionItemData;
	
	// 현재 공방 요청 QuestId
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Request")
	FName ActiveRequestId = NAME_None;
	
	// 납품 판정 결과
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Result")
	FCPPotionDeliveryResult PotionDeliveryResult;
};
