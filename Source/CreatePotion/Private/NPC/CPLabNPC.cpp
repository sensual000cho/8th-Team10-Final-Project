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
	
	for (const FName& QuestID : QuestData->LabQuestIDs){
		if (QuestID.IsNone()) continue;
		
		const FCPQuestResult QuestResult = QuestManager->GetResult(QuestID);
		if (!QuestResult.QuestId.IsNone()){
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
	if (!QuestData || QuestData->LabQuestIDs.IsEmpty() || !GetGameInstance()) { return false; }

	UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return false; }

	for (const FName& QuestID : QuestData->LabQuestIDs){
		if (QuestID.IsNone()) continue;
		if (QuestManager->GetQuestState(QuestID) == EQuestState::Accepted) return true; 
	}

	return false; 
}

void ACPLabNPC::OpenResultWidget(AActor* Interactor)
{
	if (!QuestData || QuestData->LabQuestIDs.IsEmpty()) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(World->GetAuthGameMode());
	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!UIManager || !LabGameMode || !QuestManager || !ResultWidgetClass) return;
	
	FName QuestId = NAME_None;
	for (const FName& LabQuestID : QuestData->LabQuestIDs){
		if (LabQuestID.IsNone()) continue;
		
		const FCPQuestResult DeliveryResult = QuestManager->GetResult(LabQuestID);
		if (!DeliveryResult.QuestId.IsNone()){
			QuestId = LabQuestID;
			break;
		}
	}
	
	if (QuestId.IsNone()) return;

	UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
	if (!ResultWidget) return;

	if (!ResultWidget->InitializeResult(QuestId))
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
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	
	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) return;
	
	if (!QuestData || QuestData->LabQuestIDs.IsEmpty()) return;
	
	FName QuestId = NAME_None;
	FCPQuestResult DeliveryResult = QuestManager->GetResult(QuestId);
	for (const FName& LabQuestID : QuestData->LabQuestIDs){
		if (LabQuestID.IsNone()) continue;
		
		DeliveryResult = QuestManager->GetResult(LabQuestID);
		if (!DeliveryResult.QuestId.IsNone()){
			QuestId = LabQuestID;
			break;
		}
	}
	if (QuestId.IsNone()) return;
	
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* DialogueWidgetClass = UISettings ? UISettings->NPCDialogueWidgetClass.LoadSynchronous() : nullptr;

	if (!UIManager || !QuestManager || !DialogueWidgetClass) return;

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
	if (PotionEffectTags.IsEmpty() || !QuestData || QuestData->LabQuestIDs.IsEmpty()) return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) return;
	
	FName QuestId = NAME_None;
	for (const FName& LabQuestID : QuestData->LabQuestIDs){
		if (LabQuestID.IsNone()) continue;
		
		if (QuestManager->GetQuestState(LabQuestID) == EQuestState::Accepted){
			QuestId = LabQuestID;
			break;
		}
	}
	
	if (QuestId.IsNone()) return;
	QuestManager->CreatePotionResult(QuestId, PotionEffectTags);
}

void ACPLabNPC::OnPotionReactionTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0 || !QuestData || QuestData->LabQuestIDs.IsEmpty()) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	
	if (!QuestManager) return;
	for (const FName& QuestId : QuestData->LabQuestIDs){
		if (QuestId.IsNone()) continue;
		
		const FCPQuestResult DeliveryResult = QuestManager->GetResult(QuestId);
		if (!DeliveryResult.QuestId.IsNone()){
			ShowResultDialogue();
			return;
		}
	}
}