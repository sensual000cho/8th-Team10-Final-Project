// CPGridSlotWidgetBase.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPGridSlotWidgetBase.generated.h"

class UCPItemContainerComponent;

UCLASS()
class CREATEPOTION_API UCPGridSlotWidgetBase : public UCPBaseUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;

	// 좌클릭 한 번을 통한 라우팅 기능(아이템 집기/놓기/Swap 등)은 여기서 처리
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	// 자식 클래스에서도 private 타이머를 취소시키기 위한 함수
	void CancelLeftClickCheckHandler();

	virtual int32 GetClickedSlotGridIndex(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) const { return SlotGridIndex; }

private:
	// 실제 좌클
	void ExecuteLeftClickOnce();

public:
	// 모든 슬롯(빈 슬롯 포함)이 반드시 가져야 하는 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Data")
	UCPItemContainerComponent* OwnerContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Data")
	int32 SlotGridIndex = -1;

	// 아이템을 놓을 때 클릭한 Index
	UPROPERTY(BlueprintReadOnly, Category = "Container|Data")
	int32 ClickedGridIndex = -1;

private:
	// 임계 시간동안 더블 클릭을 체크할 타이머
	FTimerHandle DoubleClickCheckHandler;
};
