#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "CPLabPotionSessionComponent.generated.h"

class UCPForageableItemData;
class ACPAlchemyProp;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCPOnLabSessionChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCPOnLabPotionResultChanged, const TArray<FGameplayTag>&, EffectTotals);

// 포션 세션과 공방 슬롯 상태를 한곳에서 관리
UCLASS()
class CREATEPOTION_API UCPLabPotionSessionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPLabPotionSessionComponent();

	// 현재 포션 결과값을 초기화
	void ResetPotionResult();
	
	// 현재 포션 결과값을 PotionProp에 저장하고 손에 든 Prop으로 등록
	bool FinalizePotionResult(ACPAlchemyProp* PotionProp, UCPForageableItemData* PotionItemData, const TArray<FGameplayTag>& PotionResult);
	
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	const TArray<FGameplayTag>& GetPotionResult() const;

public:
	// 세션이나 슬롯 상태가 바뀌었음을 Blueprint에 알림
	UPROPERTY(BlueprintAssignable, Category = "Lab|Session")
	FCPOnLabSessionChanged OnSessionChanged;
	
	// 현재 포션 결과값이 바뀌었음을 Blueprint에 알림
	UPROPERTY(BlueprintAssignable, Category = "Lab|Session")
	FCPOnLabPotionResultChanged OnPotionResultChanged;
	
private:
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Result")
	TArray<FGameplayTag> CurrentPotionResult;
};
