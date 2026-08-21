// CPItemSlotWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Common/Container/CPGridSlotWidgetBase.h"
#include "Types/CPContainerTypes.h"
#include "CPItemSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UCPItemContainerComponent;

UCLASS()
class CREATEPOTION_API UCPItemSlotWidget : public UCPGridSlotWidgetBase
{
	GENERATED_BODY()

public:
	// ContainerItem 구조체를 전달받아 업데이트 하는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void UpdateSlot(const FContainerItem& ItemData);

	// 빈 칸 상태로 초기화하는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void ClearSlot();

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	// 마우스를 (한 번만) 클릭했을 때 발생하는 엔진 자체 이벤트 처리 함수인 NativeOnMouseButtonDown는
	// 부모 클래스 UCPGridSlotWidgetBase에서 처리하도록 리팩토링

	// 마우스를 더블클릭했을 때 발생하는 엔진 자체 이벤트 처리 함수
	// 더블클릭 자체는 아이템이 존재할 때에만 의미가 있기 때문에 이 클래스에서 처리
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
public:
	// 현재 이 슬롯이 소유중인 아이템
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Slot")
	FContainerItem CachedItemData;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StackText;
};
