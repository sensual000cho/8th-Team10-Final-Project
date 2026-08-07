// CPItemSlotWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "Types/CPContainerTypes.h"
#include "CPItemSlotWidget.generated.h"

class UImage;
class UTextBlock;

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

	// virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Slot")
	FContainerItem CachedItemData;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StackText;
	
};
