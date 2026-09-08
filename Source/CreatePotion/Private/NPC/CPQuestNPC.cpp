#include "NPC/CPQuestNPC.h"

#include "AbilitySystemComponent.h"
#include "Data/NPC/CPNPCDataAsset.h"       
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"

ACPQuestNPC::ACPQuestNPC() : CurrentQuestId(NAME_None)
{
}

FText ACPQuestNPC::GetInteractionPrompt_Implementation()
{
	if (!CanInteract_Implementation(nullptr))
	{
		return FText::GetEmpty();
	}

	return FText::FromString(TEXT("대화하기"));
}

void ACPQuestNPC::HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags)
{
	if (PotionEffectTags.IsEmpty()) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;
	
	const FName QuestID = FindAcceptedQuestId();
	if (QuestID.IsNone()) return;
	
	const FCPQuestResult Result = QuestManager->CreatePotionResult(QuestID, PotionEffectTags);
	CurrentQuestId = Result.QuestId;
}

void ACPQuestNPC::OpenResultWidget(AActor* Interactor)
{
	if (CurrentQuestId.IsNone()) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	UWorld* World = GetWorld();
	if (!GameInstance || !World) return;
	
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* ResultWidgetClass = UISettings ? UISettings->ResultWidgetClass.LoadSynchronous() : nullptr;
	if (!UIManager || !ResultWidgetClass) return;
	
	UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
	if (!ResultWidget) return;
	
	if (!ResultWidget->InitializeResult(CurrentQuestId)){
		ResultWidget->RequestClose();
		return;
	}
	
	ActiveResultWidget = ResultWidget;
	CurrentQuestId = NAME_None;
}

void ACPQuestNPC::ShowResultDialogue()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;
	
	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) return;
	
	if (CurrentQuestId.IsNone()) return;
	
	FCPQuestResult DeliveryResult = QuestManager->GetResult(CurrentQuestId);
	if (DeliveryResult.QuestId.IsNone()) return;
	
	const FName QuestID = DeliveryResult.QuestId;
	
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* DialogueWidgetClass = UISettings ? UISettings->NPCDialogueWidgetClass.LoadSynchronous() : nullptr;
	if (!UIManager || !DialogueWidgetClass) return;
	
	ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
	if (!ActiveDialogueWidget) return;
	
	TArray<FText> ReactionTexts;
	const TArray<FConditionEvaluation> Evaluations = QuestManager->EvaluateConditions(DeliveryResult.QuestId, DeliveryResult.CurrentEffects);
	
	for (const FConditionEvaluation& Eval : Evaluations){
		FText Reaction = QuestManager->GetReactionText(DeliveryResult.QuestId, Eval);
		if (!Reaction.IsEmpty()){
			ReactionTexts.Add(Reaction);
		}
	}
	
	const FText ReactionText = FText::Join(FText::FromString(TEXT("\n")), ReactionTexts);
	
	ActiveDialogueWidget->InitResultDialogue(
		false, 
		QuestID, 
		FText::FromName(QuestData->NPCName), 
		ReactionText, 
		this, 
		nullptr);
}

void ACPQuestNPC::BeginPlay()
{
	Super::BeginPlay();
	
	QuestData = Cast<UCPQuestNPCDataAsset>(NPCData);
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent()){
		FGameplayTag ReactionTag = FGameplayTag::RequestGameplayTag(FName("State.Reaction.Potion"));
		ASC->RegisterGameplayTagEvent(ReactionTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ACPQuestNPC::OnPotionReactionChanged);
	}
}

const TArray<FName>& ACPQuestNPC::GetPotionQuestIDs() const
{
	static const TArray<FName> QuestIDs;
	return QuestIDs;
}

FName ACPQuestNPC::FindAcceptedQuestId() const
{
	if (!QuestData) return NAME_None;
	
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return NAME_None;
	
	for (const FName& QuestID : GetPotionQuestIDs()){
		if (QuestID.IsNone()) continue;
		if (QuestManager->GetQuestState(QuestID) != EQuestState::Accepted) continue;
		
		return QuestID;
	}
	return NAME_None;
}

void ACPQuestNPC::OnPotionReactionChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 0) return;
	
	ShowResultDialogue();
	
}
