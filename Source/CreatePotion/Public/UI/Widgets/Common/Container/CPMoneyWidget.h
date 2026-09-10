// CPMoneyWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPMoneyWidget.generated.h"

class UTextBlock;
class UCPInventoryComponent;

UCLASS()
class CREATEPOTION_API UCPMoneyWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()

public:
	// PC가 인벤토리 컴포넌트를 확보한 시점에 바인딩 호출
	void BindInventory(UCPInventoryComponent* InInventory);

private:
	UFUNCTION()
	void OnMoneyChanged(int32 NewAmount);

private:
	UPROPERTY()
	UCPInventoryComponent* BoundInventory;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoneyText;
};
