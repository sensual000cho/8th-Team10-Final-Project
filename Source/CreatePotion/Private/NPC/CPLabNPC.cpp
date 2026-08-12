#include "NPC/CPLabNPC.h"
#include "Quest/QuestManager.h"
#include "Data/CPNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "GameState/CPLabGameState.h" 
#include "Lab/Component/CPLabPotionSessionComponent.h"

// UI 연결 전 디버깅용
namespace
{
	FString GetDeliveryGradeText(EDeliveryGrade DeliveryGrade)
	{
		switch (DeliveryGrade){
		case EDeliveryGrade::Fail:
			return TEXT("Fail");

		case EDeliveryGrade::Okay:
			return TEXT("Okay");

		case EDeliveryGrade::Good:
			return TEXT("Good");

		case EDeliveryGrade::Perfect:
			return TEXT("Perfect");

		default:
			return TEXT("Unknown");
		}
	}

	void ShowPotionDeliveryResultDebugMessage(const FCPPotionDeliveryResult& Result)
	{
		if (!GEngine) return;

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Yellow,
			FString::Printf(
				TEXT("납품 결과 | QuestId: %s | Grade: %s | Reward: %d | Tip: %d"),
				*Result.QuestId.ToString(),
				*GetDeliveryGradeText(Result.DeliveryGrade),
				Result.RewardAmount,
				Result.TipAmount));

		for (const FAlchemyProperty& CurrentEffect : Result.CurrentEffects){
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Cyan,
				FString::Printf(
					TEXT("현재 효과 | %s : %d"),
					*CurrentEffect.Tag.ToString(),
					CurrentEffect.Value));
		}

		for (int32 Index = 0; Index < Result.MinTargetEffects.Num(); ++Index){
			const FAlchemyProperty& MinTargetEffect = Result.MinTargetEffects[Index];

			FString MaxValueText = TEXT("None");
			if (Result.MaxTargetEffects.IsValidIndex(Index)){
				MaxValueText = FString::FromInt(Result.MaxTargetEffects[Index].Value);
			}

			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Green,
				FString::Printf(
					TEXT("목표 효과 | %s : Min %d / Max %s"),
					*MinTargetEffect.Tag.ToString(),
					MinTargetEffect.Value,
					*MaxValueText));
		}
	}
}

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
	if (UWorld* World = GetWorld()){
		if (ACPLabGameState* LabState = World->GetGameState<ACPLabGameState>()){
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
			
			// 결과 UI가 없어 구조체 값을 화면에 출력한다(UI 대체 이후 삭제)
			const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult();
			ShowPotionDeliveryResultDebugMessage(DeliveryResult);

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
			// 결과 UI 구현 전 테스트용: 결과 출력 후 바로 납품 확인 처리(UI 대체 이후 삭제)
			LabGameMode->ConfirmPotionDeliveryResult();
			return;
		}

		FText FirstHint = QuestManager->GetSessionHintText(QuestID);

		if (DialogueWidgetClass)
		{
			if (UCPNPCDialogueWidget* DialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidgetBP(DialogueWidgetClass)))
			{
				FText NPCNameText = FText::FromName(NPCData->NPCName);
				DialogueWidget->InitDialogue(true, QuestID, NPCNameText, FirstHint, this);
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