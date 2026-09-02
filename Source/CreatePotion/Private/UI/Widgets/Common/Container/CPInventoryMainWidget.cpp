// CPInventoryMainWidget.cpp

#include "UI/Widgets/Common/Container/CPInventoryMainWidget.h"
#include "Character/CPPlayerController.h"
#include "Components/CPInventoryComponent.h"

void UCPInventoryMainWidget::HandleCloseRequested()
{
	if (ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayer()))
	{
		if (PC->CachedInventoryComponent)
		{
			PC->CachedInventoryComponent->CloseInventoryUI();
		}
	}
}
