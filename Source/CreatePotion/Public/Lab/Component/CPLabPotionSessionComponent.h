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

	UFUNCTION(BlueprintPure, Category = "Lab|Session")
	bool HasActiveRequest() const;

	UFUNCTION(BlueprintPure, Category = "Lab|Request")
	bool GetActiveRequestState(FCPLabPotionRequestState& OutRequestState) const;
	
	// 전달받은 리퀘스트들로 새 포션 세션 시작
	bool StartRequest(const FCPLabPotionRequest& PotionRequest);

	// 진행 중인 세션과 슬롯을 모두 초기 상태로 되돌림
	void ResetRequest();

	// 정해진 진행 순서에 맞을 때만 리퀘스트 상태 변경
	bool SetRequestPhase(ECPLabPotionRequestPhase NewPhase);
	
	// 플레이어가 들고 있는 재료 Prop 참조를 가져옴
	UFUNCTION(BlueprintPure, Category = "Lab|Carry")
	ACPAlchemyProp* GetHeldAlchemyProp() const;
	
	// 현재 들고 있는 재료가 있는지 확인
	UFUNCTION(BlueprintPure, Category = "Lab|Carry")
	bool HasHeldAlchemyProp() const;
	
	// 재료 Prop을 현재 들고 있는 재료로 등록
	UFUNCTION(BlueprintCallable, Category = "Lab|Carry")
	bool HoldAlchemyProp(ACPAlchemyProp* AlchemyProp);
	
	// 들고 있는 재료 Prop 참조를 꺼내고 보유 상태를 비움
	UFUNCTION(BlueprintCallable, Category = "Lab|Carry")
	bool ReleaseHeldAlchemyProp(ACPAlchemyProp*& OutAlchemyProp);
	
	// 현재 포션 결과값을 PotionProp에 저장하고 손에 든 Prop으로 등록
	bool FinalizePotionResult(ACPAlchemyProp* PotionProp, UCPForageableItemData* PotionItemData);
	
	UFUNCTION(BlueprintCallable, Category = "Lab|Result")
	const TArray<FGameplayTag>& GetPotionResult() const;

private:
	// 현재 상태에서 요청한 다음 상태로 이동할 수 있는지 확인
	bool CanTransitionRequestPhase(ECPLabPotionRequestPhase NewPhase) const;

public:
	// 세션이나 슬롯 상태가 바뀌었음을 Blueprint에 알림
	UPROPERTY(BlueprintAssignable, Category = "Lab|Session")
	FCPOnLabSessionChanged OnSessionChanged;
	
	// 현재 포션 결과값이 바뀌었음을 Blueprint에 알림
	UPROPERTY(BlueprintAssignable, Category = "Lab|Session")
	FCPOnLabPotionResultChanged OnPotionResultChanged;
	
private:
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Request")
	FCPLabPotionRequestState ActiveRequestState;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Request")
	bool bHasActiveRequest;
	
	// 현재 들고 있는 재료 Prop
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Carry")
	TObjectPtr<ACPAlchemyProp> HeldAlchemyProp;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Lab|Result")
	TArray<FGameplayTag> CurrentPotionResult;
};
