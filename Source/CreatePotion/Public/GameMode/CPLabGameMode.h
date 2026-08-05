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
	
	// Waiting 상태일 때 Lab 세션을 시작하는 함수 -> 이제는 WithRequest 호출 위한 Wrapper로 변경
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	bool TryStartLabSession();
	
	//LabSession 시작할 때 리퀘스트 확인
	UFUNCTION(BlueprintCallable, Category = "Lab|Session")
	bool TryStartLabSessionWithRequest(const FCPLabRequest& Request);
	
	// 현재 요청이 유효하고 Request 단계일 때 Preparing으로 전환
	UFUNCTION(BlueprintCallable, Category = "Lab|Request")
	bool TryAcceptLabRequest();
	
	//Lab Session 페이즈 제대로 넘어가는지 체킹용 디버깅 함수
	UFUNCTION(BlueprintCallable, Category = "Lab|Debug")
	void DebugAdvanceSessionPhase();
	
private:
	// 엔진이 생성한 GameState를 Lab 타입으로 가져오는, 캐스팅 과정 절약용 함수
	ACPLabGameState* GetLabGameState() const;
	bool SetSessionPhase(ECPLabSessionPhase NewPhase);	
	
	//테스트용 더미 리퀘스트
	UPROPERTY(EditDefaultsOnly, Category = "Lab|Request")
	FCPLabRequest DefaultTestRequest;
};
