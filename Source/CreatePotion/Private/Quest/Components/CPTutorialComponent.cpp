#include "Quest/Components/CPTutorialComponent.h"

#include "Components/CPInventoryComponent.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"

UCPTutorialComponent::UCPTutorialComponent()
{
	InteractionPrompt = FText::FromString(TEXT("F : 읽기"));
	TargetName = FName(TEXT("TutorialNote"));
}

void UCPTutorialComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;

	const EQuestState State = QuestManager->GetQuestState(AssociatedQuestID);
	if ((!bIsCompletionActor && State != EQuestState::NotAccepted)
		|| (bIsCompletionActor && State == EQuestState::Completed))
	{
		if (AActor* Owner = GetOwner())
		{
			Owner->Destroy();
		}
	}
}

bool UCPTutorialComponent::CanInteract() const
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return false;

	const EQuestState State = QuestManager->GetQuestState(AssociatedQuestID);
	if (State == EQuestState::NotAccepted)
	{
		return QuestManager->IsQuestStoryUnlocked(AssociatedQuestID);
	}

	if (State != EQuestState::Accepted) return false;

	const EQuestCompletionType Type = QuestManager->GetQuestCompletionType(AssociatedQuestID);
	return Type == EQuestCompletionType::Dialogue || Type == EQuestCompletionType::ItemCollection;
}

bool UCPTutorialComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor)) return false;

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GameInstance) return false;

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	if (!QuestManager || !UIManager || !DialogueWidgetClass) return false;

	const EQuestState State = QuestManager->GetQuestState(AssociatedQuestID);
	if (State == EQuestState::NotAccepted)
	{
		if (!QuestManager->IsQuestStoryUnlocked(AssociatedQuestID)) return false;

		const TArray<FText> ScriptLines = QuestManager->GetQuestScriptLines(AssociatedQuestID);
		UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
		if (Widget)
		{
			Widget->InitDialogueLines(AssociatedQuestID, DisplayName, ScriptLines);
			QuestManager->AcceptQuest(AssociatedQuestID);
		}

		if (AActor* Owner = GetOwner())
		{
			Owner->Destroy();
		}
		return true;
	}

	if (State != EQuestState::Accepted) return false;

	const EQuestCompletionType Type = QuestManager->GetQuestCompletionType(AssociatedQuestID);
	if (Type == EQuestCompletionType::Dialogue)
	{
		const TArray<FText> StoryLines = QuestManager->GetNPCStoryLines(AssociatedQuestID);
		if (StoryLines.IsEmpty() || !QuestManager->CompleteQuestByDialogue(AssociatedQuestID)) return false;

		if (UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass)))
		{
			Widget->InitDialogueLines(AssociatedQuestID, DisplayName, StoryLines);
		}

		if (AActor* Owner = GetOwner())
		{
			Owner->Destroy();
		}
		return true;
	}

	if (Type != EQuestCompletionType::ItemCollection || !IsValid(Interactor)) return false;

	UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
	if (!Inventory) return false;

	const TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();
	if (!QuestManager->TryCompleteQuestByItem(AssociatedQuestID, HeldItems)) return false;

	TArray<FName> RequiredIDs;
	int32 RequiredCount = 0;
	QuestManager->GetRequiredItemInfo(AssociatedQuestID, RequiredIDs, RequiredCount);
	for (const FName& ItemID : RequiredIDs)
	{
		Inventory->RemoveItemByID(ItemID, RequiredCount);
	}

	const TArray<FText> StoryLines = QuestManager->GetNPCStoryLines(AssociatedQuestID);
	if (!StoryLines.IsEmpty())
	{
		if (UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass)))
		{
			Widget->InitDialogueLines(AssociatedQuestID, DisplayName, StoryLines);
		}
	}

	if (AActor* Owner = GetOwner())
	{
		Owner->Destroy();
	}
	return true;
}
