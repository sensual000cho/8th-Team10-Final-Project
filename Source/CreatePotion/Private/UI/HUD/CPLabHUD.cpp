// CPLabHUD.cpp

#include "UI/HUD/CPLabHUD.h"

#include "Character/CPPlayerController.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "UI/Widgets/Lab/TagChoice/CPTagRangeWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagSelectionWidget.h"


void ACPLabHUD::OnMainHUDWidgetCreated()
{
	Super::OnMainHUDWidgetCreated();
	
	//TODO: 퀘스트 저널 토글 델리게이트 바인딩
}

void ACPLabHUD::StartLabCraftingFlow()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager || !TagSelectionWidgetClass) return;
	
	UCPTagSelectionWidget* CreatedWidget = Cast<UCPTagSelectionWidget>(UIManager->FindOpenWidget(TagSelectionWidgetClass));
	if (CreatedWidget)
	{
		if (UCPTagSelectionWidget* SelectionWidget = Cast<UCPTagSelectionWidget>(CreatedWidget))
		{
			SelectionWidget->OnTagSelectionConfirmed.AddDynamic(this, &ACPLabHUD::HandleTagSelectionConfirmed);
			UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 태그 선택 확정 델리게이트 바인딩 성공!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 태그 선택 위젯 바인딩 실패 - 위젯을 찾지 못함"));
	}
	
}

void ACPLabHUD::BindTagSelectionWidget(UCPTagSelectionWidget* TargetWidget)
{
	if (TargetWidget)
	{
		TargetWidget->OnTagSelectionConfirmed.AddUniqueDynamic(this, &ACPLabHUD::HandleTagSelectionConfirmed);
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] TagSelectionWidget 델리게이트 바인딩 완료"))
	}
}

void ACPLabHUD::BindTagRangeWidget(UCPTagRangeWidget* TargetWidget)
{
	if (TargetWidget)
	{
		TargetWidget->OnTagRangeConfirmed.AddUniqueDynamic(this, &ACPLabHUD::HandleTagRangeConfirmed);
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] TagRangeWidget 델리게이트 바인딩 완료"))
	}
}

void ACPLabHUD::HandleTagRangeConfirmed()
{
	UE_LOG(LogTemp, Warning, TEXT("[LabHUD] HandleTagConfirmed 호출됨"));
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager) return;
	UE_LOG(LogTemp, Warning, TEXT("[HUD] 태그 범위 설정 완료"));
	
	if (ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayerController()))
	{
		if (PC->CurrentInteractingContainer)
		{
			PC->ToggleExternalContainerUI(PC->CurrentInteractingContainer);
			UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 재료 선택 UI 열기 성공"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 재료 선택 UI 열기 성공"));
	}
	
	
}

void ACPLabHUD::HandleTagSelectionConfirmed(const FTagSelectionData& SelectionData)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager || !TagRangeWidgetClass) return;
	
	UUserWidget* CreatedWidget = UIManager->PushWidget(TagRangeWidgetClass);
	
	if (UCPTagRangeWidget* RangeWidget = Cast<UCPTagRangeWidget>(CreatedWidget))
	{
		RangeWidget->InitTagRangeWidget(
			SelectionData.QuestID,
			SelectionData.SelectedTags,
			SelectionData.SavedValues
		);
		
		BindTagRangeWidget(RangeWidget);
		
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 태그 범위 위젯 오픈 및 초기화 완료!"));
	}
}

void ACPLabHUD::HandleQuestJournalToggle()
{
	UE_LOG(LogTemp, Warning, TEXT("[LabHUD] HandleQuestJournalToggle 호출됨"));
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager) return;
	
	if (QuestJournalWidgetClass)
	{
		UIManager->ToggleWidget(QuestJournalWidgetClass);	
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[LabHUD] QuestJournalWidgetClass 미할당"));
	}
}
