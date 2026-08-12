#include "NPC/CPLabNPC.h"
#include "Quest/QuestManager.h"
#include "Data/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"
#include "GameState/CPLabGameState.h" 
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "GameplayTagContainer.h"
#include "Components/StateTreeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Character/CPInteractionComponent.h"

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
		const bool bHasRequestState = SessionComp && SessionComp->GetRequestState(QuestID, RequestState);

		// PotionReady 상태에서 NPC와 다시 상호작용 하면 GameMode를 통해 납품 과정을 수행한다
		if (bRequestConfirmed && bHasRequestState && RequestState.Phase == ECPLabPotionRequestPhase::PotionReady)
		{
			ACPLabGameMode* LabGameMode = GetWorld() ? Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;
			if (!LabGameMode) { return; }

			// 판정 결과를 저장
			if (!LabGameMode->TryDeliverActivePotion()) { return; }

			/* TODO 결과 UI로 대체
			결과 UI를 열고, 결과 UI에서 GetPotionDeliveryResult()로 표시 데이터를 가져간다.
			결과 UI 확인 버튼에서 ConfirmPotionDeliveryResult()를 호출한다.
			*/
			const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult();

			FText ResultDialogueText;
			FName EventTagName = NAME_None;

			switch (DeliveryResult.DeliveryGrade)
			{
			case EDeliveryGrade::Perfect:
				ResultDialogueText = FText::FromString(TEXT("와!정말 마음에 드는 포션이에요! 정말 감사합니다."));
				//EventTagName = FName("NPC.Event.DeliverySuccess");
				break;
			case EDeliveryGrade::Good:
				ResultDialogueText = FText::FromString(TEXT("와! 너무 괜찮네요~ 고생하셨어요"));
				//EventTagName = FName("NPC.Event.DeliverySuccess");
				break;

			case EDeliveryGrade::Okay:
				ResultDialogueText = FText::FromString(TEXT("음... 요청한 성분과 약간 차이가 있지만, 그런대로 쓸 수 있겠네요."));
				//EventTagName = FName("NPC.Event.DeliveryPartialFail");
				break;

			case EDeliveryGrade::Fail:
			default:
				ResultDialogueText = FText::FromString(TEXT("이건 제가 부탁한 포션이 전혀 아니에요! 다시 만들어 주시겠어요?"));
				//EventTagName = FName("NPC.Event.DeliveryFail");
				break;
			}

			// 대화 위젯 띄우기
			if (DialogueWidgetClass)
			{
				ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidgetBP(DialogueWidgetClass));

				if (ActiveDialogueWidget)
				{
					FText NPCNameText = FText::FromName(NPCData->NPCName);
					ActiveDialogueWidget->InitResultDialogue(true, QuestID, NPCNameText, ResultDialogueText, this);
				}
			}

			/* StateTree 애니메이션 태그 송출 
			if (StateTreeComponent && !EventTagName.IsNone())
			{
				FStateTreeEvent Event;
				Event.Tag = FGameplayTag::RequestGameplayTag(EventTagName);
				StateTreeComponent->SendStateTreeEvent(Event);
			}
			*/
			break;
		}
		FText FirstHint = QuestManager->GetSessionHintText(QuestID);

		// 포션 준비 단계 아니라면 기존 힌트 대화 출력
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

void ACPLabNPC::OpenResultWidget()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	ACPLabGameMode* LabGameMode = GetWorld() ? Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;

	if (!UIManager || !LabGameMode || !ResultWidgetClass) return;

	const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult();
	UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidgetBP(ResultWidgetClass));

	if (!ResultWidget)
	{
		return;
	}

	if (!ResultWidget->InitializeResult(DeliveryResult))
	{
		ResultWidget->RequestClose();
		return;
	}

	ActiveResultWidget = ResultWidget;

	// 결과 버튼들 바인딩
	ResultWidget->OnConfirmRequested.AddUniqueDynamic(this, &ACPLabNPC::HandleResultAccepted);
	ResultWidget->OnContinueRequested.AddUniqueDynamic(this, &ACPLabNPC::HandleResultAccepted);
	ResultWidget->OnRetryRequested.AddUniqueDynamic(this, &ACPLabNPC::HandleResultRetryRequested);
}

//넘어가기 버튼 클릭 시 (퀘스트 완료 처리 및 NPC 사라짐)
void ACPLabNPC::HandleResultAccepted()
{
	ACPLabGameMode* LabGameMode = GetWorld() ? Cast<ACPLabGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;

	if (!LabGameMode)
	{
		return;
	}

	if (!LabGameMode->ConfirmPotionDeliveryResult())
	{
		return;
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
	if (IsValid(ActiveResultWidget))
	{
		ActiveResultWidget->RequestClose();
	}
	ActiveResultWidget = nullptr;

	//아직 구현 안되어 있음. 더미 확인용
	UE_LOG(LogTemp, Warning, TEXT("재시도 요청: 아직 재시도 상태 전환 기능이 구현되지 않았습니다."));
}