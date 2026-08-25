#include "NPC/CPLabNPC.h"

#include "Quest/QuestManager.h"
#include "Data/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Actor/CPPotionActor.h"

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
		if (QuestID.IsNone()) continue;

		EQuestState CurrentState = QuestManager->GetQuestState(QuestID);
		if (CurrentState != EQuestState::Accepted) continue;

		// QuestId에 대응되는 리퀘스트 상태를 조회한다
		FCPLabPotionRequestState RequestState;
		ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode());
		const bool bHasRequestState = LabGameMode && LabGameMode->GetActiveRequestId() == QuestID;
		
		FText FirstHint = QuestManager->GetSessionHintText(QuestID);

		// 포션 준비 단계 아니라면 기존 힌트 대화 출력
		if (DialogueWidgetClass)
		{
			ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));

			if (ActiveDialogueWidget)
			{
				FText NPCNameText = FText::FromName(NPCData->NPCName);
				TArray<FText> DialogueLines;
				DialogueLines.Add(FirstHint);

				ActiveDialogueWidget->InitDialogueLines(true,QuestID,NPCNameText,DialogueLines,this);
			}
		}
		break;
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
	
	return true;
}

void ACPLabNPC::OpenResultWidget(AActor* Interactor)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	
	UWorld* World = GetWorld();
	if (!World) return;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode());
	if (!UIManager || !LabGameMode || !ResultWidgetClass) return;

	const FName QuestId = LabGameMode->GetActiveRequestId();
	if (QuestId.IsNone()) return;
	
	const FCPPotionDeliveryResult DeliveryResult = PendingThrownPotionDeliveryResult;
	if (DeliveryResult.QuestId.IsNone()) return;
	
	UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
	if (!ResultWidget) return;

	if (!ResultWidget->InitializeResult(DeliveryResult)){
		ResultWidget->RequestClose();
		return;
	}
	
	LabGameMode->AdvancePotionRequest();
	ActiveResultWidget = ResultWidget;
	PendingThrownPotionDeliveryResult = FCPPotionDeliveryResult{};
	
	// NPC 반응 추가 시 삭제
	Destroy();
}

void ACPLabNPC::HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags)
{
	if (PotionEffectTags.IsEmpty() || !NPCData || NPCData->LabQuestIDs.IsEmpty()) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	UWorld* World = GetWorld();
	if (!GameInstance || !World) return;
	
	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(World->GetAuthGameMode());
	if (!QuestManager || !UIManager || !LabGameMode || !DialogueWidgetClass) return;
	
	const FName QuestId = LabGameMode->GetActiveRequestId();
	if (QuestId.IsNone() || !NPCData->LabQuestIDs.Contains(QuestId)) return;
	if (QuestManager->GetQuestState(QuestId) != EQuestState::Accepted) return;
	
	// 결과 계산 및 저장
	FCPPotionDeliveryResult DeliveryResult;
	DeliveryResult.QuestId = QuestId;
	DeliveryResult.CurrentEffects = PotionEffectTags;
	DeliveryResult.DeliveryGrade = QuestManager->TryDeliver(QuestId, DeliveryResult.CurrentEffects);
	DeliveryResult.RewardAmount = QuestManager->GetRewardGold(QuestId);
	DeliveryResult.TipAmount = 0;
	
	PendingThrownPotionDeliveryResult = DeliveryResult;
	ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
	if (!ActiveDialogueWidget) return;
	
	FText NPCNameText = FText::FromName(NPCData->NPCName);
	ActiveDialogueWidget->InitResultDialogue(
		false,
		QuestId,
		NPCNameText,
		QuestManager->GetQuestSummaryText(QuestId),
		this,
		nullptr);
}

//넘어가기 버튼 클릭 시 (퀘스트 완료 처리 및 NPC 사라짐)
void ACPLabNPC::HandleResultAccepted()
{
	//퀘스트 완료 처리
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>())
		{
			for (const FName& QuestID : NPCData->LabQuestIDs)
			{
				if (!QuestID.IsNone() && QuestManager->GetQuestState(QuestID) == EQuestState::Accepted)
				{
					QuestManager->CompleteQuest(QuestID);
					break;
				}
			}
		}
	}
	if (IsValid(ActiveResultWidget))
	{
		ActiveResultWidget->RequestClose();
	}

	ActiveResultWidget = nullptr;
	Destroy();
}