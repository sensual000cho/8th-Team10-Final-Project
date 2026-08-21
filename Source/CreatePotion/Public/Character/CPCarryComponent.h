// CPCarryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CPCarryComponent.generated.h"

class ACPThrowablePropBase;

/**
 * 플레이어가 들고 있는 Prop의 부착과 투척을 관리한다.
 *
 * 현재 재료의 세션 보유 상태는 CPLabPotionSessionComponent와
 * 동기화하여 사용한다.
 */
UCLASS(ClassGroup = (Carry), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPCarryComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCPCarryComponent();

	// Prop을 이 컴포넌트 위치에 부착
	UFUNCTION(BlueprintCallable, Category = "Carry")
	bool AttachProp(ACPThrowablePropBase* Prop);

	// Prop을 분리하고 월드에 배치
	UFUNCTION(BlueprintCallable, Category = "Carry")
	bool DetachProp(ACPThrowablePropBase* Prop, const FVector& DropLocation);

	UFUNCTION(BlueprintPure, Category = "Carry")
	bool HasHeldProp() const;

	UFUNCTION(BlueprintPure, Category = "Carry")
	ACPThrowablePropBase* GetHeldProp() const;

	/*
	 * 임시 테스트용 재료 투척 함수.
	 *
	 * Processing 단계인지 확인하고,
	 * PotionSession에서 재료를 해제한 뒤 투척한다.
	 *
	 * 이후 GA_ThrowIngredient가 만들어지면
	 * 세션 검사는 Ability로 이동.
	 */
	UFUNCTION(BlueprintCallable, Category = "Carry|Throw")
	bool TryThrowHeldAlchemyProp(float ThrowSpeed = 800.f, float UpwardBias = 0.2f);

	// 공통 Prop 물리 투척
	bool ThrowHeldProp(const FVector& Direction, float Speed);

private:
	UPROPERTY()
	TObjectPtr<ACPThrowablePropBase> HeldProp;
};
