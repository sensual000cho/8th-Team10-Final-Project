#include "NPC/CPLabNPC.h"
#include "Quest/QuestManager.h"
#include "Data/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"

void ACPLabNPC::OnInteract_Implementation(AActor* Interactor)
{
	if (!NPCData || NPCData->LabQuestIDs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] DA에 지정된 공방 퀘스트가 없습니다."), *GetName());
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) { return; }

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return; }

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager) { return; }

	for (const FName& QuestID : NPCData->LabQuestIDs)
	{
		if (QuestID.IsNone()) { continue; }

		EQuestState CurrentState = QuestManager->GetQuestState(QuestID);

		if (CurrentState == EQuestState::Accepted)
		{
			FText FirstHint = QuestManager->GetSessionHintText(QuestID);

			if (DialogueWidgetClass)
			{
				if (UCPNPCDialogueWidget* DialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidgetBP(DialogueWidgetClass)))
				{
					FText NPCNameText = FText::FromName(NPCData->NPCName);
					DialogueWidget->InitDialogue(true, QuestID, NPCNameText, FirstHint);
				}
			}
			break;
		}
	}
}

bool ACPLabNPC::CanInteract_Implementation(AActor* Interactor)
{
	if (!NPCData || !GetGameInstance()) { return false; }

	UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return false; }

	for (const FName& QuestID : NPCData->LabQuestIDs)
	{
		if (QuestID.IsNone()) { continue; }
		if (QuestManager->GetQuestState(QuestID) == EQuestState::Accepted)
		{
			int32 CurrentHintLevel = QuestManager->GetQuestHintLevel(QuestID);

			// 힌트 단계가 2(마지막) 미만일 때만 상호작용 가능
			if (CurrentHintLevel < 2)
			{
				return true;
			}
		}
	}
	return false;

}