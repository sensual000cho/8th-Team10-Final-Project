// CPItemSlotWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "Types/CPContainerTypes.h"
#include "CPItemSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UCPItemContainerComponent;

UCLASS()
class CREATEPOTION_API UCPItemSlotWidget : public UCPBaseUserWidget
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

	// 마우스를 (한 번만) 클릭했을 때 발생하는 엔진 자체 이벤트 처리 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 마우스를 한 번만 클릭했을 때 실제로 실행될 함수
	void ExecuteLeftClickOnce();

	// 마우스를 더블클릭했을 때 발생하는 엔진 자체 이벤트 처리 함수
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
public:
	// 현재 이 슬롯을 소유중인 컨테이너
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container|Data")
	UCPItemContainerComponent* OwnerContainer;

	// 현재 이 슬롯이 소유중인 아이템
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Slot")
	FContainerItem CachedItemData;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StackText;

private:
	// 임계 시간동안 더블 클릭을 체크할 타이머
	FTimerHandle DoubleClickCheckHandler;
};
