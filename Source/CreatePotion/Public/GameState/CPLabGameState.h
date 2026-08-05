// CPLabGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameState/CPGameStateBase.h"
#include "Lab/CPLabTypes.h"
#include "CPLabGameState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FCPOnLabSessionPhaseChanged,
	ECPLabSessionPhase,
	NewPhase);
UCLASS()
class CREATEPOTION_API ACPLabGameState : public ACPGameStateBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Lab|Session")
	ECPLabSessionPhase GetCurrentPhase() const { return CurrentPhase; }
	
	//공방 세션 단계가 변경되면 등록된 구독 액터에게 새 단계 알림
	UPROPERTY(BlueprintAssignable, Category = "Lab|Session")
	FCPOnLabSessionPhaseChanged OnSessionPhaseChanged;
	
	// 프로젝트 규칙상 LabGameMode에서만 써야 하는 함수. 상태 변경된 경우 true 반환.
	bool ApplySessionPhase(ECPLabSessionPhase NewPhase);

private:
	UPROPERTY()	ECPLabSessionPhase CurrentPhase = ECPLabSessionPhase::WaitingForBell;	
};
