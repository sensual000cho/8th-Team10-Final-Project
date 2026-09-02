#include "NPC/CPLabNPC.h"
#include "Quest/QuestManager.h"
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"
#include "Settings/CPUISettings.h"
#include "AbilitySystemComponent.h"

void ACPLabNPC::BeginPlay()
{
	Super::BeginPlay();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayTag ReactionTag = FGameplayTag::RequestGameplayTag(FName("State.Reaction.Potion"));
		ASC->RegisterGameplayTagEvent(ReactionTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ACPLabNPC::OnPotionReactionTagChanged);
	}
}

void ACPLabNPC::OnInteract_Implementation(AActor* Interactor)
{
	if (ActiveDialogueWidget && ActiveDialogueWidget->IsInViewport())
	{
		return;
	}

	const UCPQuestNPCDataAsset* QuestData = GetQuestNPCData();
	if (!QuestData || QuestData->LabQuestIDs.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] DA에 지정된 공방 퀘스트가 없습니다."), *GetName());
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) { return; }

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	if (!QuestManager || !UIManager) { return; }

	UWorld* World = GetWorld();
	ACPLabGameMode* LabGameMode = World? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;
	if (LabGameMode){
		const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult();
		
		if (!DeliveryResult.QuestId.IsNone()){
			ShowResultDialogue();
			return;
		}
	}
	
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* DialogueWidgetClass = UISettings ? UISettings->NPCDialogueWidgetClass.LoadSynchronous() : nullptr;
	if (!DialogueWidgetClass) { return; }

	for (const FName& QuestID : QuestData->LabQuestIDs)
	{
		if (QuestID.IsNone()) continue;

		EQuestState CurrentState = QuestManager->GetQuestState(QuestID);
		if (CurrentState != EQuestState::Accepted) continue;

		FText FirstHint = QuestManager->GetSessionHintText(QuestID);
		ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));

		if (ActiveDialogueWidget)
		{
			FText NPCNameText = FText::FromName(QuestData->NPCName);
			TArray<FText> DialogueLines;
			DialogueLines.Add(FirstHint);

			ActiveDialogueWidget->InitDialogueLines(true, QuestID, NPCNameText, DialogueLines, this);
		}

		break;
	}
}

bool ACPLabNPC::CanInteract_Implementation(AActor* Interactor)
{
	const UCPQuestNPCDataAsset* QuestData = GetQuestNPCData();
	if (!QuestData || QuestData->LabQuestIDs.IsEmpty() || !GetGameInstance()) { return false; }

	UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return false; }

	FName QuestID = QuestData->LabQuestIDs[0];
	if (QuestID.IsNone()) { return false; }

	return QuestManager->GetQuestState(QuestID) == EQuestState::Accepted;
}

void ACPLabNPC::OpenResultWidget(AActor* Interactor)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(World->GetAuthGameMode());
	if (!UIManager || !LabGameMode || !ResultWidgetClass) return;

	const FName QuestId = LabGameMode->GetActiveRequestId();
	if (QuestId.IsNone()) return;

	const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult();
	if (DeliveryResult.QuestId.IsNone()) return;

	UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
	if (!ResultWidget) return;

	if (!ResultWidget->InitializeResult())
	{
		ResultWidget->RequestClose();
		return;
	}

	LabGameMode->ClearPotionRequest();
	ActiveResultWidget = ResultWidget;

	Destroy();
}

void ACPLabNPC::ShowResultDialogue()
{
	UWorld* World = GetWorld();
	ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;
	if (!LabGameMode) return;
	
	const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult();
	if (DeliveryResult.QuestId.IsNone()) return;

	const UCPQuestNPCDataAsset* QuestData = GetQuestNPCData();
	if (!QuestData) return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* DialogueWidgetClass = UISettings ? UISettings->NPCDialogueWidgetClass.LoadSynchronous() : nullptr;

	if (!UIManager || !QuestManager || !DialogueWidgetClass) return;

	const FName QuestId = DeliveryResult.QuestId;

	ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
	if (!ActiveDialogueWidget) return;

	FText NPCNameText = FText::FromName(QuestData->NPCName);
	TArray<FConditionEvaluation> Evaluations = QuestManager->EvaluateConditions(QuestId, DeliveryResult.CurrentEffects);

	FConditionEvaluation SelectedEval;
	bool bFoundEval = false;

	if (DeliveryResult.DeliveryGrade == EDeliveryGrade::Perfect)
	{
		for (const FConditionEvaluation& Eval : Evaluations)
		{
			if (Eval.Result == EConditionMatchResult::Correct)
			{
				SelectedEval = Eval;
				bFoundEval = true;
				break;
			}
		}
	}
	else
	{
		for (const FConditionEvaluation& Eval : Evaluations)
		{
			if (Eval.Result == EConditionMatchResult::WrongTag)
			{
				SelectedEval = Eval;
				bFoundEval = true;
				break;
			}
		}
	}

	FText ReactionText;
	if (bFoundEval)
	{
		ReactionText = QuestManager->GetReactionText(QuestId, SelectedEval);
	}

	ActiveDialogueWidget->InitResultDialogue(
		false,
		QuestId,
		NPCNameText,
		ReactionText,
		this,
		nullptr);
}

void ACPLabNPC::HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags)
{
	const UCPQuestNPCDataAsset* QuestData = GetQuestNPCData();
	if (PotionEffectTags.IsEmpty() || !QuestData || QuestData->LabQuestIDs.IsEmpty()) return;

	UGameInstance* GameInstance = GetGameInstance();
	UWorld* World = GetWorld();
	if (!GameInstance || !World) return;

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(World->GetAuthGameMode());

	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* DialogueWidgetClass = UISettings ? UISettings->NPCDialogueWidgetClass.LoadSynchronous() : nullptr;

	if (!QuestManager || !UIManager || !LabGameMode || !DialogueWidgetClass) return;

	const FName QuestId = LabGameMode->GetActiveRequestId();
	if (QuestId.IsNone() || !QuestData->LabQuestIDs.Contains(QuestId)) return;
	if (QuestManager->GetQuestState(QuestId) != EQuestState::Accepted) return;

	LabGameMode->GetPotionDeliveryResult(PotionEffectTags);
}

void ACPLabNPC::OnPotionReactionTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0) return;
	
	UWorld* World = GetWorld();
	ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;
	if (!LabGameMode) return;
	
	const FCPPotionDeliveryResult DeliveryResult = LabGameMode->GetPotionDeliveryResult();
	
	if (!DeliveryResult.QuestId.IsNone()){
		ShowResultDialogue();
	}
}