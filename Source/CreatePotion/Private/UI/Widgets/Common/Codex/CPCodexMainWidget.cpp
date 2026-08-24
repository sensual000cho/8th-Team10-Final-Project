// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPCodexMainWidget.h"
#include "UI/Widgets/Common/Codex/CPForageableCodexWidget.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"

void UCPCodexMainWidget::BindEvents()
{
	Super::BindEvents();
	
	if (!CodexSubsystem)
	{
		UGameInstance* GI = GetGameInstance();
		if (!GI) return;
		CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	}
	
	CodexSubsystem->OnCodexItemSelected.AddUniqueDynamic(this, &UCPCodexMainWidget::OpenItemDetail);
}

void UCPCodexMainWidget::UnbindEvents()
{
	if (CodexSubsystem)
	{
		CodexSubsystem->OnCodexItemSelected.RemoveDynamic(this, &UCPCodexMainWidget::OpenItemDetail);	
	}
}

void UCPCodexMainWidget::OpenItemDetail(UCPForageableItemData* SelectedItemData)
{
	UE_LOG(LogTemp, Warning, TEXT("[CodexMain] OpenItemDetail 호출됨! 클래스 유효성: %s"), 
		ForageableCodexWidgetClass ? TEXT("유효함(O)") : TEXT("비어있음(X)"));
	
	UE_LOG(LogTemp, Warning, TEXT("[CodexMainWidget] OpenItemDetail 호출됨"));
	if (!ForageableCodexWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[CodexMainWidget] ForageableCodexWidgetClass 가 nullptr입니다."));
	}
	UCPForageableCodexWidget* DetailWidget = 
		Cast<UCPForageableCodexWidget>(CachedUIManager->PushWidget(ForageableCodexWidgetClass));
	
	if (!DetailWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[CodexMainWidget] 아이템 도감 위젯이 설정되어있지 않습니다."))
		return;
	}
	
	DetailWidget->SetForageableItem(SelectedItemData);
}
