// CPContainerMainWidget.cpp

#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "CreatePotion.h"   // 로그용 헤더
#include "UI/Widgets/Common/Container/CPContainerGridWidget.h"
#include "Components/CPItemContainerComponent.h"

void UCPContainerMainWidget::BindContainer(UCPItemContainerComponent* InContainer)
{
	if (!InContainer || !ContainerGrid)
	{
		return;
	}

	TargetContainer = InContainer;

	// 연결된 컴포넌트의 Grid(Column * Row) 또는 Slot(개수)에 맞춰 Grid 세팅
	ContainerGrid->InitializeGrid(TargetContainer);

	// 아이템 Container내부 배열이 변경되면 델리게이트 Broadcast 예정, 수신시 UI 업데이트
	TargetContainer->OnContainerUpdated.AddDynamic(this, &UCPContainerMainWidget::UpdateUI);

	// 세팅이 끝났으면 UI 업데이트
	UpdateUI();
}

void UCPContainerMainWidget::UpdateUI()
{
	if (TargetContainer && ContainerGrid)
	{
		ContainerGrid->UpdateGrid(TargetContainer->ContainerItems);
	}
}
