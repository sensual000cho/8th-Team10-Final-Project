// CPLabHUD.cpp

#include "UI/HUD/CPLabHUD.h"

#include "Character/CPPlayerController.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"

void ACPLabHUD::OnMainHUDWidgetCreated()
{
	Super::OnMainHUDWidgetCreated();
	
	//TODO: 퀘스트 저널 토글 델리게이트 바인딩
}

void ACPLabHUD::StartLabCraftingFlow()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
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
