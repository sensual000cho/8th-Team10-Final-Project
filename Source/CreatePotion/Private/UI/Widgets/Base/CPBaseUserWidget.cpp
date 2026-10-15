// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"

void UCPBaseUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindEvents();
	CachedUIManager = GetGameInstance()->GetSubsystem<UCPUIManagerSubsystem>();
}

void UCPBaseUserWidget::NativeDestruct()
{
	UnbindEvents();
	Super::NativeDestruct();
}

FReply UCPBaseUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	
	// 버튼이나 자식이 처리한 경우
	if (Reply.IsEventHandled()) return Reply;
	
	// 처리되지 않은 좌클릭이 IA_Look으로 전달되는 것을 막는다.
	if (bConsumeLeftMouseButton && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton){
		return FReply::Handled();
	}
	
	return Reply;
}

void UCPBaseUserWidget::BindEvents()
{
	
}

void UCPBaseUserWidget::UnbindEvents()
{
	
}

bool UCPBaseUserWidget::HandleEscape()
{
	return false;
}
