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
	
public:
	// 모든 슬롯(빈 슬롯 포함)이 반드시 가져야 하는 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Data")
	UCPItemContainerComponent* OwnerContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Data")
	int32 GridIndex = -1;

protected:
	// 클릭을 통한 라우팅(아이템 집기/놓기/Swap 등)은 여기서 처리
	// virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
