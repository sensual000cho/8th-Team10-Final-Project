#include "NPC/CPLabNPC.h"
#include "Quest/QuestManager.h"
#include "Data/CPNPCDataAsset.h"

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

	for (const FName& QuestID : NPCData->LabQuestIDs)
	{
		if (QuestID.IsNone()) { continue; }

		EQuestState CurrentState = QuestManager->GetQuestState(QuestID);

		if (CurrentState == EQuestState::Accepted)
		{
			FText FirstHint = QuestManager->GetSessionHintText(QuestID);
			UE_LOG(LogTemp, Warning, TEXT("[%s 공방 1차 힌트 - QuestID: %s]: %s"),
				*NPCData->NPCName.ToString(),
				*QuestID.ToString(),
				*FirstHint.ToString());

			// TODO: 실제 UI 대화창에 FirstHint 띄우기 (버튼 이벤트 처리는 WBP에서 진행)
			break;
		}
	}
}

bool ACPLabNPC::CanInteract_Implementation(AActor* Interactor)
{
	/* 리퀘스트 확인을 위해 일단 주석 처리
	if (!NPCData || !GetGameInstance()) { return false; }

	UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return false; }

	for (const FName& QuestID : NPCData->LabQuestIDs)
	{
		if (QuestID.IsNone()) { continue; }
		if (QuestManager->GetQuestState(QuestID) == EQuestState::Accepted)
		{
			return true;
		}
	}

	return false;
	*/
	return true;
}

FText ACPLabNPC::GetInteractionPrompt_Implementation()
{
	if (!CanInteract_Implementation(nullptr))
	{
		return FText::GetEmpty();
	}
	return Super::GetInteractionPrompt_Implementation();
}