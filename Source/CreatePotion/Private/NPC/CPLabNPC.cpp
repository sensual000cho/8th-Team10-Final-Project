#include "NPC/CPLabNPC.h"

#include "Character/CPCarryComponent.h"
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

		// PotionReady 상태에서 NPC와 다시 상호작용 하면 GameMode를 통해 납품 과정을 수행한다
		const UCPCarryComponent* CarryComponent = IsValid(Interactor)
			? Interactor->FindComponentByClass<UCPCarryComponent>() : nullptr;
		const ACPPotionActor* HeldPotion = CarryComponent ? Cast<ACPPotionActor>(CarryComponent->GetHeldProp()) : nullptr;
		
		if (bRequestConfirmed && bHasRequestState && IsValid(HeldPotion))
		{
			// 대화 위젯 띄우기
			if (DialogueWidgetClass)
			{
				ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));

				if (ActiveDialogueWidget)
				{
					FText NPCNameText = FText::FromName(NPCData->NPCName);
					ActiveDialogueWidget->InitResultDialogue(
						true,
						QuestID,
						NPCNameText,
						QuestManager->GetQuestSummaryText(QuestID),
						this,
						Interactor);
				}
			}
			break;
		}
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
	if (QuestManager->GetQuestHintLevel(QuestID) < 2)
	{
		return true;
	}
	if (!bRequestConfirmed)
	{
		return true;
	}
	
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode());
	if (!LabGameMode || LabGameMode->GetActiveRequestId() != QuestID) return false;
	
	const UCPCarryComponent* CarryComponent = IsValid(Interactor)
	? Interactor->FindComponentByClass<UCPCarryComponent>()
	: nullptr;

	return CarryComponent && IsValid(Cast<ACPPotionActor>(CarryComponent->GetHeldProp()));
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

	const UCPCarryComponent* CarryComponent = IsValid(Interactor)
	? Interactor->FindComponentByClass<UCPCarryComponent>()
	: nullptr;

	ACPPotionActor* HeldPotion = CarryComponent
		? Cast<ACPPotionActor>(CarryComponent->GetHeldProp())
		: nullptr;

	if (!IsValid(HeldPotion)) return;
	
	const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult(QuestId, HeldPotion);
	
	if (DeliveryResult.QuestId.IsNone()) return;
	
	UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
	if (!ResultWidget) return;

	if (!ResultWidget->InitializeResult(DeliveryResult))
	{
		ResultWidget->RequestClose();
		return;
	}

	HeldPotion->Destroy();
	
	LabGameMode->AdvancePotionRequest();
	ActiveResultWidget = ResultWidget;
	
	// NPC 반응 추가 시 삭제
	Destroy();
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

// 재시도 버튼 클릭 시 (결과창만 닫고 NPC는 유지)
void ACPLabNPC::HandleResultRetryRequested()
{
	// TODO : 재시도 

	if (IsValid(ActiveResultWidget))
	{
		ActiveResultWidget->RequestClose();
	}
	ActiveResultWidget = nullptr;

	//아직 구현 안되어 있음. 더미 확인용
	UE_LOG(LogTemp, Warning, TEXT("재시도 요청: 아직 재시도 상태 전환 기능이 구현되지 않았습니다."));
}