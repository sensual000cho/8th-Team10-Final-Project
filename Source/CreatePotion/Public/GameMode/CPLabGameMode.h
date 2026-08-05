// CPLabGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameMode/CPGameModeBase.h"
#include "Lab/CPLabTypes.h"
#include "CPLabGameMode.generated.h"

class ACPLabGameState;

UCLASS()
class CREATEPOTION_API ACPLabGameMode : public ACPGameModeBase
{
	GENERATED_BODY()
public:
	ACPLabGameMode();
	
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	void ResetLabSession();
	
	// Waiting 상태일 때 Lab 세션을 시작하는 함수
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	bool TryStartLabSession();
	
	//Lab Session 페이즈 제대로 넘어가는지 체킹용 디버깅 함수
	UFUNCTION(BlueprintCallable, Category = "Lab|Debug")
	void DebugAdvanceSessionPhase();
	
private:
	// 엔진이 생성한 GameState를 Lab 타입으로 가져오는, 캐스팅 과정 절약용 함수
	ACPLabGameState* GetLabGameState() const;
	bool SetSessionPhase(ECPLabSessionPhase NewPhase);	
};
