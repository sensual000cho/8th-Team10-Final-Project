#include "NPC/CPLabNPC.h"
#include "Quest/QuestManager.h"
#include "Data/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "GameState/CPLabGameState.h" 
#include "Lab/Component/CPLabPotionSessionComponent.h"

void ACPLabNPC::OnInteract_Implementation(AActor* Interactor)
{
	if (ActiveDialogueWidget && ActiveDialogueWidget->IsInViewport())
	{
		return;
	} 

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
				ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidgetBP(DialogueWidgetClass));

				if (ActiveDialogueWidget)
				{
					FText NPCNameText = FText::FromName(NPCData->NPCName);
					ActiveDialogueWidget->InitDialogue(true, QuestID, NPCNameText, FirstHint, this);
				}
			}
			break;
		}
	}
}

bool ACPLabNPC::CanInteract_Implementation(AActor* Interactor)
{
	if (!NPCData || NPCData->LabQuestIDs.IsEmpty() || !GetGameInstance()) { return false; }
	UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return false; }
	FName QuestID = NPCData->LabQuestIDs[0];
	if (QuestID.IsNone()) { return false; }
	if (QuestManager->GetQuestState(QuestID) != EQuestState::Accepted) {
		return false;
	}

	if (!bRequestConfirmed)
	{
		return true;
	}

	if (UWorld* World = GetWorld()) {
		if (ACPLabGameState* LabState = World->GetGameState<ACPLabGameState>()) {
			if (UCPLabPotionSessionComponent* SessionComp = LabState->GetPotionSession()) {
				FCPLabPotionRequestState RequestState;
				if (SessionComp->GetRequestState(QuestID, RequestState)) {
					return RequestState.Phase == ECPLabPotionRequestPhase::PotionReady;
				}
			}
		}
	}

	return false;
}