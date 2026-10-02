#include "Quest/Components/CPTutorialComponent.h"

#include "Components/CPInventoryComponent.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "Quest/Components/CPQuestMarkerComponent.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"

UCPTutorialComponent::UCPTutorialComponent()
{
	InteractionPrompt = FText::FromString(TEXT("F : 읽기"));
	TargetName = FName(TEXT("TutorialNote"));
}

void UCPTutorialComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			QuestManager->OnQuestUpdated.AddDynamic(this, &UCPTutorialComponent::HandleQuestUpdated);
		}
	}

	RefreshStep();
}

void UCPTutorialComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			QuestManager->OnQuestUpdated.RemoveDynamic(this, &UCPTutorialComponent::HandleQuestUpdated);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UCPTutorialComponent::HandleQuestUpdated(FName QuestID, EQuestState NewState)
{
	RefreshStep();
}

void UCPTutorialComponent::RefreshStep()
{
	AActor* Owner = GetOwner();
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UQuestManager* QuestManager = GI ? GI->GetSubsystem<UQuestManager>() : nullptr;
	if (!Owner || !QuestManager) return;

	const FCPTutorialStep* ActiveStep = nullptr;
	for (const FCPTutorialStep& Step : Steps)
	{
		if (Step.QuestID.IsNone()) continue;

		const EQuestState State = QuestManager->GetQuestState(Step.QuestID);
		const bool bPending = Step.bIsCompletionStep
			? State == EQuestState::Accepted
			: (State == EQuestState::NotAccepted && QuestManager->IsQuestStoryUnlocked(Step.QuestID));

		if (bPending)
		{
			ActiveStep = &Step;
			break;
		}
	}

	ActiveQuestID = ActiveStep ? ActiveStep->QuestID : NAME_None;

	if (UCPQuestMarkerComponent* Marker = Owner->FindComponentByClass<UCPQuestMarkerComponent>())
	{
		Marker->SetTrackedQuest(ActiveQuestID);
	}

	// 진행할 단계가 없으면 삭제하지 않고 숨김
	const bool bActive = ActiveStep != nullptr;
	Owner->SetActorHiddenInGame(!bActive);
	Owner->SetActorEnableCollision(bActive);

	if (bActive)
	{
		if (AActor* Point = ActiveStep->TargetPoint.Get())
		{
			Owner->SetActorLocation(Point->GetActorLocation());
		}
		RefreshUI();
	}
}

bool UCPTutorialComponent::CanInteract() const
{
	if (ActiveQuestID.IsNone()) return false;

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return false;

	const EQuestState State = QuestManager->GetQuestState(ActiveQuestID);
	if (State == EQuestState::NotAccepted)
	{
		return QuestManager->IsQuestStoryUnlocked(ActiveQuestID);
	}

	if (State != EQuestState::Accepted) return false;

	const EQuestCompletionType Type = QuestManager->GetQuestCompletionType(ActiveQuestID);
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

	// 수락/완료 시 OnQuestUpdated로 ActiveQuestID가 바뀌므로 지금 값을 복사해서 사용
	const FName QuestID = ActiveQuestID;

	const EQuestState State = QuestManager->GetQuestState(QuestID);
	if (State == EQuestState::NotAccepted)
	{
		if (!QuestManager->IsQuestStoryUnlocked(QuestID)) return false;

		const TArray<FCPDialogueLine> ScriptLines = QuestManager->GetQuestScriptLines(QuestID);
		UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
		if (!Widget) return false;

		Widget->InitDialogueLines(QuestID, DisplayName, ScriptLines);
		QuestManager->AcceptQuest(QuestID);
		return true;
	}

	if (State != EQuestState::Accepted) return false;

	const EQuestCompletionType Type = QuestManager->GetQuestCompletionType(QuestID);
	if (Type == EQuestCompletionType::Dialogue)
	{
		const TArray<FCPDialogueLine> StoryLines = QuestManager->GetNPCStoryLines(QuestID);
		if (StoryLines.IsEmpty() || !QuestManager->CompleteQuestByDialogue(QuestID)) return false;

		if (UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass)))
		{
			Widget->InitDialogueLines(QuestID, DisplayName, StoryLines);
		}
		return true;
	}

	if (Type != EQuestCompletionType::ItemCollection || !IsValid(Interactor)) return false;

	UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
	if (!Inventory) return false;

	const TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();
	if (!QuestManager->TryCompleteQuestByItem(QuestID, HeldItems)) return false;

	if (QuestManager->ShouldConsumeRequiredItems(QuestID))
	{
		TArray<FName> RequiredIDs;
		int32 RequiredCount = 0;
		QuestManager->GetRequiredItemInfo(QuestID, RequiredIDs, RequiredCount);
		for (const FName& ItemID : RequiredIDs)
		{
			Inventory->RemoveItemByID(ItemID, RequiredCount);
		}
	}

	const TArray<FCPDialogueLine> StoryLines = QuestManager->GetNPCStoryLines(QuestID);
	if (!StoryLines.IsEmpty())
	{
		if (UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass)))
		{
			Widget->InitDialogueLines(QuestID, DisplayName, StoryLines);
		}
	}
	return true;
}