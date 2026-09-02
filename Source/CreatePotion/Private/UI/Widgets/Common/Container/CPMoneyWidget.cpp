// CPMoneyWidget.cpp

#include "UI/Widgets/Common/Container/CPMoneyWidget.h"
#include "Components/TextBlock.h"

#include "Components/CPInventoryComponent.h"

void UCPMoneyWidget::BindInventory(UCPInventoryComponent* InInventory)
{
	if (!InInventory || BoundInventory == InInventory)
	{
		return; // 이미 같은 대상에 바인딩되어 있으면 중복 델리게이트 등록 방지
	}

	BoundInventory = InInventory;
	BoundInventory->OnMoneyChanged.AddDynamic(this, &UCPMoneyWidget::OnMoneyChanged);

	OnMoneyChanged(BoundInventory->GetOwningMoney()); // 최초 값 즉시 반영
}

void UCPMoneyWidget::OnMoneyChanged(int32 NewAmount)
{
	if (MoneyText)
	{
		MoneyText->SetText(FText::AsNumber(NewAmount));
	}
}
