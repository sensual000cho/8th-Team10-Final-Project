#include "NPC/CPTownNPC.h"
#include "Quest/QuestManager.h"
#include "Data/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "GameplayTagContainer.h"
#include "Components/StateTreeComponent.h"

void ACPTownNPC::OnInteract_Implementation(AActor* Interactor)
{
	if (ActiveDialogueWidget && ActiveDialogueWidget->IsInViewport())
	{
		return;
	}
	if (!NPCData || NPCData->TownQuestIDs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] DA에 지정된 마을 퀘스트가 없습니다."), *GetName());
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) { return; }

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return; }

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager) { return; }

	// DA(TownQuestIDs)에 지정된 퀘스트 ID들만 순회
	for (const FName& QuestID : NPCData->TownQuestIDs)
	{
		if (QuestID.IsNone()) { continue; }

		EQuestState CurrentState = QuestManager->GetQuestState(QuestID);

		// DA에 등록된 퀘스트 중 NotAccepted 첫번째 퀘스트 발견 시
		if (CurrentState == EQuestState::NotAccepted)
		{
			FText FullScript = QuestManager->GetQuestFullText(QuestID);
			UE_LOG(LogTemp, Log, TEXT("[%s 마을대화 - QuestID: %s]: %s"),
				*NPCData->NPCName.ToString(),
				*QuestID.ToString(),
				*FullScript.ToString());
			if (DialogueWidgetClass)
			{
				ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));

				if (ActiveDialogueWidget)
				{
					FText NPCNameText = FText::FromName(NPCData->NPCName);
					ActiveDialogueWidget->InitDialogue(false, QuestID, NPCNameText, FullScript);
				}
			}
			if (StateTreeComponent)
			{
				FStateTreeEvent Event;
				Event.Tag = FGameplayTag::RequestGameplayTag(FName("NPC.Event.Talk"));
				StateTreeComponent->SendStateTreeEvent(Event);
			}
			break;
		}
	}
}

bool ACPTownNPC::CanInteract_Implementation(AActor* Interactor)
{
	if (!NPCData || !GetGameInstance()) { return false; }

	UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return false; }

	// NPC가 가진 마을 퀘스트 중 NotAccepted 퀘스트가 있는지 검사
	for (const FName& QuestID : NPCData->TownQuestIDs)
	{
		if (QuestID.IsNone())
		{
			continue;
		}

		if (QuestManager->GetQuestState(QuestID) == EQuestState::NotAccepted)
		{
			return true;
		}
	}

	// 모든 퀘스트를 수락했다면 상호작용 불가 상태 (false)로 만듦
	return false;
}
