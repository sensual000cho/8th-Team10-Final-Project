#pragma once

#include "CoreMinimal.h"
#include "GameMode/CPGameModeBase.h"
#include "GameplayTagContainer.h"
#include "CPLabGameMode.generated.h"

class UCPForageableItemData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPotionRequestStateChanged, bool);

// Blueprint와 월드 Actor가 포션 세션 기능을 호출하는 입구
UCLASS()
class CREATEPOTION_API ACPLabGameMode : public ACPGameModeBase
{
	GENERATED_BODY()

public:
	ACPLabGameMode();

	// 포션 리퀘스트 시작
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool StartPotionRequest();
	
	// 포션 리퀘스트 종료 & 정리
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool ClearPotionRequest();
	
	// 포션 리퀘스트 상태 반환
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool IsActiveRequest() const;
	
	// Tags 사전 순 정렬
	UFUNCTION(BlueprintCallable, Category = "Lab|Potion")
	bool CreatePotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform, const FVector& SpawnImpulse);
	
private:	
	// Potion Spawn
	bool SpawnPotion(const TArray<FGameplayTag>& EffectTags, const FTransform& SpawnTransform, const FVector& SpawnImpulse);

public:
	FOnPotionRequestStateChanged OnPotionRequestStateChanged;
	
private:
	// 포션 Prop을 만들 때 사용할 DA
	UPROPERTY(EditDefaultsOnly, Category = "Lab|Potion")
	TObjectPtr<UCPForageableItemData> PotionItemData;
	
	// 포션 리퀘스트 상태
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Request")
	bool bActiveRequest;
};
