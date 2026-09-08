#include "NPC/CPTownNPC.h"
#include "Quest/QuestManager.h"
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Components/CPInventoryComponent.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"

void ACPTownNPC::OnInteract_Implementation(AActor* Interactor)
{
	if (ActiveDialogueWidget && ActiveDialogueWidget->IsInViewport())
	{
		return;
	}
	const UCPQuestNPCDataAsset* QuestData = GetQuestNPCData();
	if (!QuestData || QuestData->TownQuestIDs.Num() == 0)
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
	for (const FName& QuestID : QuestData->TownQuestIDs)
	{
		if (QuestID.IsNone()) { continue; }
		EQuestState CurrentState = QuestManager->GetQuestState(QuestID);
		// DA에 등록된 퀘스트 중 NotAccepted 첫번째 퀘스트 발견 시
		if (CurrentState == EQuestState::NotAccepted)
		{

			if (!QuestManager->IsQuestStoryUnlocked(QuestID))
			{
				continue;
			}

			TArray<FText> ScriptLines = QuestManager->GetQuestScriptLines(QuestID);

			// TODO: InitDialogue가 배열을 받도록 논의 후 교체 예정
			// 임시로 한 줄씩 합쳐서 기존 FText 시그니처 유지
			// FText FullScript = FText::Join(FText::FromString(TEXT(" ")), ScriptLines);

			UE_LOG(LogTemp, Log, TEXT("[%s 마을대화 - QuestID: %s, %d줄]"),
				*QuestData->NPCName.ToString(),
				*QuestID.ToString(),
				ScriptLines.Num());
			if (DialogueWidgetClass)
			{
				ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
				if (ActiveDialogueWidget)
				{
					FText NPCNameText = FText::FromName(QuestData->NPCName);
					// [변경] InitDialogue(FText 버전) → InitDialogueLines(TArray<FText> 버전)
					// 여러 줄 대사를 순서대로 타이핑해서 보여주기 위해 배열을 그대로 전달
					ActiveDialogueWidget->InitDialogueLines(false, QuestID, NPCNameText, ScriptLines);
				}
			}
			break;
		}

		else if (CurrentState == EQuestState::Accepted
			&& QuestManager->GetQuestCompletionType(QuestID) == EQuestCompletionType::Dialogue)
		{
			TArray<FText> StoryLines = QuestManager->GetNPCStoryLines(QuestID);
			if (StoryLines.Num() > 0 && DialogueWidgetClass)
			{
				ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
				if (ActiveDialogueWidget)
				{
					FText NPCNameText = FText::FromName(QuestData->NPCName);
					// bIsWorkshopQuest를 false로 넘겨서, InitDialogueLines 내부의
					// bCurrentIsWorkshopQuest 분기(힌트+스토리 자동조합)를 타지 않고
					// StoryLines 배열을 있는 그대로 재생하도록 함
					ActiveDialogueWidget->InitDialogueLines(false, QuestID, NPCNameText, StoryLines);
				}
			}
			break;
		}

		else if (CurrentState == EQuestState::Accepted
			&& QuestManager->GetQuestCompletionType(QuestID) == EQuestCompletionType::ItemCollection)
		{
			UCPInventoryComponent* Inventory = Interactor ? Interactor->FindComponentByClass<UCPInventoryComponent>() : nullptr;
			if (Inventory)
			{
				TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();

				if (QuestManager->TryCompleteQuestByItem(QuestID, HeldItems))
				{
					TArray<FName> RequiredIDs;
					int32 RequiredCount = 0;
					QuestManager->GetRequiredItemInfo(QuestID, RequiredIDs, RequiredCount);

					for (const FName& ItemID : RequiredIDs)
					{
						Inventory->RemoveItemByID(ItemID, RequiredCount);
					}

					UE_LOG(LogTemp, Log, TEXT("[%s] 아이템 수집 퀘스트 [%s] 완료 처리됨"),
						*QuestData->NPCName.ToString(), *QuestID.ToString());

					// 신규 추가 - 완료 성공 대사를 대화창으로 보여줌
					TArray<FText> StoryLines = QuestManager->GetNPCStoryLines(QuestID);
					if (StoryLines.Num() > 0 && DialogueWidgetClass)
					{
						ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
						if (ActiveDialogueWidget)
						{
							FText NPCNameText = FText::FromName(QuestData->NPCName);
							ActiveDialogueWidget->InitDialogueLines(false, QuestID, NPCNameText, StoryLines);
						}
					}
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("[%s] 아이템 수집 퀘스트 [%s] 아직 조건 미충족"),
						*QuestData->NPCName.ToString(), *QuestID.ToString());
				}
			}
			break;
		}

	}
}

bool ACPTownNPC::CanInteract_Implementation(AActor* Interactor)
{
	const UCPQuestNPCDataAsset* QuestData = GetQuestNPCData();
	if (!QuestData || !GetGameInstance()) { return false; }

	UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return false; }

	// NPC가 가진 마을 퀘스트 중 NotAccepted 퀘스트가 있는지 검사
	for (const FName& QuestID : QuestData->TownQuestIDs)
	{
		if (QuestID.IsNone())
		{
			continue;
		}

		EQuestState CurrentState = QuestManager->GetQuestState(QuestID);

		if (CurrentState == EQuestState::NotAccepted
			&& QuestManager->IsQuestStoryUnlocked(QuestID))
		{
			return true;
		}

		// Accepted 상태에서 완료 확정 대화/아이템 확인이 필요한 경우도 상호작용 가능해야 함
		if (CurrentState == EQuestState::Accepted)
		{
			EQuestCompletionType Type = QuestManager->GetQuestCompletionType(QuestID);
			if (Type == EQuestCompletionType::Dialogue || Type == EQuestCompletionType::ItemCollection)
			{
				return true;
			}
		}
	}

	// 모든 퀘스트를 수락했다면 상호작용 불가 상태 (false)로 만듦
	return false;
}
