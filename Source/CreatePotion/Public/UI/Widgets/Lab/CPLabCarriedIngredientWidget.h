#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Lab/CPLabIngredientInfoWidget.h"
#include "CPLabCarriedIngredientWidget.generated.h"

class UCPCarryComponent;
class ACPThrowablePropBase;

// 플레이어가 현재 운반 중인 재료를 보여주는 고정 HUD 카드다.
UCLASS()
class CREATEPOTION_API UCPLabCarriedIngredientWidget : public UCPLabIngredientInfoWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;

private:
	UFUNCTION()
	void HandlePropChanged(ACPThrowablePropBase* Prop);

	// CarryComponent의 HeldProp 변경 알림을 받기 위해 런타임 동안만 보관한다.
	UPROPERTY(Transient)
	TObjectPtr<UCPCarryComponent> BoundCarryComponent;
};
