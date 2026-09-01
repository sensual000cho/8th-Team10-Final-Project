#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Lab/CPLabIngredientInfoWidget.h"
#include "CPLabCarriedIngredientWidget.generated.h"

class UCPCarryComponent;
class ACPThrowablePropBase;
class UCPInteractionComponent;

/**
 * 플레이어가 현재 운반 중인 재료를 보여주는 고정 HUD 카드다.
 * 운반 재료의 데이터 출처와 빈손이어도 항상 보이게 해 준다.
 */
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

	UFUNCTION()
	void HandleInteractionFocusChanged(FText Prompt, FName TargetName, ECPInteractionDisplayState DisplayState);

	UFUNCTION()
	void HandlePreviewIngredientChanged();

	void BindPreviewProp(ACPThrowablePropBase* Prop);

	// CarryComponent의 HeldProp 변경 알림을 받기 위해 런타임 동안만 보관한다.
	UPROPERTY(Transient)
	TObjectPtr<UCPCarryComponent> BoundCarryComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<UCPInteractionComponent> BoundInteractionComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<ACPThrowablePropBase> HeldProp;
};
