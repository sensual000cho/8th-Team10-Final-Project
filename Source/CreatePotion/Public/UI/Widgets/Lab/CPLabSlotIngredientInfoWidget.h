#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Lab/CPLabIngredientInfoWidget.h"
#include "CPLabSlotIngredientInfoWidget.generated.h"

/**
 * 현재 가리키는 공방 슬롯의 재료를 보여주는 호버 카드다.
 * 슬롯 선택, 세션 변경 갱신, 호버 중에만 보이는 정책을 담당한다.
 */
UCLASS()
class CREATEPOTION_API UCPLabSlotIngredientInfoWidget : public UCPLabIngredientInfoWidget
{
	GENERATED_BODY()

public:
	// 슬롯 Actor가 하이라이트될 때 호출하며, 슬롯 번호만 받아 세션에서 재료를 조회한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void BeginSlotHover(int32 InSlotIndex);

	// 이전 슬롯의 종료 이벤트가 새 슬롯 카드를 숨기지 않도록 슬롯 번호를 함께 받는다.
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void EndSlotHover(int32 InSlotIndex);

protected:
	virtual void NativeConstruct() override;

private:
	void RefreshHoveredSlot();
	void HideSlotInfo();

	// 현재 카드가 추적 중인 슬롯이다. 초기값은 INDEX_NONE으로. 
	int32 HoveredSlotIndex = INDEX_NONE;
};
