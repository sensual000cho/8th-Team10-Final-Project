// CPHUDBase.cpp

#include "UI/HUD/CPHUDBase.h"
#include "Blueprint/UserWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"

void ACPHUDBase::BeginPlay()
{
	Super::BeginPlay();
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	CachedUIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!CachedUIManager) return;
	
	if (MainHUDWidgetClass)
	{
		CachedUIManager->PushWidget(MainHUDWidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] MainHUDWidgetClass가 지정되지 않았습니다."), *GetName());
	}
	
	OnMainHUDWidgetCreated();
}

void ACPHUDBase::OnMainHUDWidgetCreated()
{

}
