// CPHUDBase.cpp

#include "UI/HUD/CPHUDBase.h"
#include "Blueprint/UserWidget.h"

void ACPHUDBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (!MainHUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] MainHUDWidgetClass가 지정되지 않았습니다."), *GetName());
		return;
	}
	
	MainHUDWidget = CreateWidget<UUserWidget>(GetOwningPlayerController(), MainHUDWidgetClass);
	if (MainHUDWidget)
	{
		MainHUDWidget->AddToViewport(0);
		OnMainHUDWidgetCreated();
	}
}

void ACPHUDBase::OnMainHUDWidgetCreated()
{
	
}
