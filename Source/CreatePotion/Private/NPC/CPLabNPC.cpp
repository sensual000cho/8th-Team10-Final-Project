#include "NPC/CPLabNPC.h"
#include "Quest/QuestManager.h"
#include "Data/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"
#include "GameState/CPLabGameState.h" 
#include "Lab/Actor/CPAlchemyProp.h"
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

	// NPC가 담당하는 리퀘스트의 공방 Phase를 확인하기 이해 세션 컴포넌트를 가져온다
	UCPLabPotionSessionComponent* SessionComp = nullptr;
	if (UWorld* World = GetWorld()) {
		if (ACPLabGameState* LabState = World->GetGameState<ACPLabGameState>()) {
			SessionComp = LabState->GetPotionSession();
		}
	}

	for (const FName& QuestID : NPCData->LabQuestIDs)
	{
		if (QuestID.IsNone()) continue;

		EQuestState CurrentState = QuestManager->GetQuestState(QuestID);
		if (CurrentState != EQuestState::Accepted) continue;

		// QuestId에 대응되는 리퀘스트 상태를 조회한다
		FCPLabPotionRequestState RequestState;
		const bool bHasRequestState = SessionComp &&
			SessionComp->GetActiveRequestState(RequestState) &&
			RequestState.PotionRequest.RequestId == QuestID;

		// PotionReady 상태에서 NPC와 다시 상호작용 하면 GameMode를 통해 납품 과정을 수행한다
		if (bRequestConfirmed && bHasRequestState && RequestState.Phase == ECPLabPotionRequestPhase::PotionReady)
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
						RequestState.PotionRequest.DisplayText,
						this);
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

	UWorld* World = GetWorld();
	if (!World) return false;
	
	ACPLabGameState* LabGameState = World->GetGameState<ACPLabGameState>();
	if (!LabGameState) return false;
	
	UCPLabPotionSessionComponent* SessionComp = LabGameState->GetPotionSession();
	if (!SessionComp) return false;
	
	FCPLabPotionRequestState RequestState;
	if (!SessionComp->GetActiveRequestState(RequestState) ||
		RequestState.PotionRequest.RequestId != QuestID) {
		return false;
	}
	
	return RequestState.Phase == ECPLabPotionRequestPhase::PotionReady && SessionComp->HasHeldAlchemyProp();
}

void ACPLabNPC::OpenResultWidget()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	
	UWorld* World = GetWorld();
	if (!World) return;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode());
	ACPLabGameState* LabGameState = World->GetGameState<ACPLabGameState>();
	UCPLabPotionSessionComponent* SessionComponent = LabGameState ? LabGameState->GetPotionSession() : nullptr;

	if (!UIManager || !LabGameMode || !SessionComponent ||!ResultWidgetClass) return;

	FCPLabPotionRequestState RequestState;
	if (!SessionComponent->GetActiveRequestState(RequestState) || RequestState.Phase != ECPLabPotionRequestPhase::PotionReady) return;

	ACPAlchemyProp* HeldPotion = SessionComponent->GetHeldAlchemyProp();
	if (!IsValid(HeldPotion)) return;
	
	const FName QuestId = RequestState.PotionRequest.RequestId;
	const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult(QuestId, HeldPotion);
	
	if (DeliveryResult.QuestId.IsNone()) return;
	
	UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
	if (!ResultWidget) return;

	if (!ResultWidget->InitializeResult(DeliveryResult))
	{
		ResultWidget->RequestClose();
		return;
	}

	ACPAlchemyProp* DeliveredPotion = nullptr;
	if (SessionComponent->ReleaseHeldAlchemyProp(DeliveredPotion) && IsValid(DeliveredPotion)){
		DeliveredPotion->Destroy();
	}
	
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