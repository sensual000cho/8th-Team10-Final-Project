// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/CPTutorialActor.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "Components/CPInteractableComponent.h"
#include "Components/CPInventoryComponent.h"

ACPTutorialActor::ACPTutorialActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultRoot;

	InteractableComponent = CreateDefaultSubobject<UCPInteractableComponent>(TEXT("InteractableComponent"));
}

void ACPTutorialActor::BeginPlay()
{
	Super::BeginPlay();

	UQuestManager* QuestManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UQuestManager>() : nullptr;
	if (QuestManager)
	{
		EQuestState State = QuestManager->GetQuestState(AssociatedQuestID);

		if (!bIsCompletionActor && State != EQuestState::NotAccepted)
		{
			
			Destroy();
			return;
		}

		if (bIsCompletionActor && State == EQuestState::Completed)
		{
		 
			Destroy();
			return;
		}
	}

	RefreshInteractability();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			InteractionRefreshTimerHandle, this,
			&ACPTutorialActor::RefreshInteractability, 0.5f, true);
	}
}

void ACPTutorialActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InteractionRefreshTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ACPTutorialActor::RefreshInteractability()
{
	if (!InteractableComponent) return;

	UQuestManager* QuestManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager)
	{
		return;
	}

	EQuestState State = QuestManager->GetQuestState(AssociatedQuestID);

	bool bCanInteract = false;
	if (State == EQuestState::NotAccepted)
	{
		bCanInteract = QuestManager->IsQuestStoryUnlocked(AssociatedQuestID);
	}
	else if (State == EQuestState::Accepted)
	{
		EQuestCompletionType Type = QuestManager->GetQuestCompletionType(AssociatedQuestID);
		bCanInteract = (Type == EQuestCompletionType::Dialogue || Type == EQuestCompletionType::ItemCollection);
	}

	
}

void ACPTutorialActor::HandleInteract(AActor* Interactor)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	if (!QuestManager || !UIManager || !DialogueWidgetClass) return;

	EQuestState State = QuestManager->GetQuestState(AssociatedQuestID);

	if (State == EQuestState::NotAccepted)
	{
		if (!QuestManager->IsQuestStoryUnlocked(AssociatedQuestID)) return;

		TArray<FText> ScriptLines = QuestManager->GetQuestScriptLines(AssociatedQuestID);

		UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
		if (Widget)
		{
			Widget->InitDialogueLines(AssociatedQuestID, DisplayName, ScriptLines);
			QuestManager->AcceptQuest(AssociatedQuestID);
		}

		Destroy();
		return;
	}

	if (State == EQuestState::Accepted)
	{
		EQuestCompletionType Type = QuestManager->GetQuestCompletionType(AssociatedQuestID);

		if (Type == EQuestCompletionType::Dialogue)
		{
			TArray<FText> StoryLines = QuestManager->GetNPCStoryLines(AssociatedQuestID);
			if (StoryLines.Num() > 0 && QuestManager->CompleteQuestByDialogue(AssociatedQuestID))
			{
				UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
				if (Widget)
				{
					Widget->InitDialogueLines(AssociatedQuestID, DisplayName, StoryLines);
				}
				Destroy();
			}
		}
		else if (Type == EQuestCompletionType::ItemCollection)
		{
			UCPInventoryComponent* Inventory = Interactor ? Interactor->FindComponentByClass<UCPInventoryComponent>() : nullptr;
			if (Inventory)
			{
				TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();

				if (QuestManager->TryCompleteQuestByItem(AssociatedQuestID, HeldItems))
				{
					TArray<FName> RequiredIDs;
					int32 RequiredCount = 0;
					QuestManager->GetRequiredItemInfo(AssociatedQuestID, RequiredIDs, RequiredCount);

					for (const FName& ItemID : RequiredIDs)
					{
						Inventory->RemoveItemByID(ItemID, RequiredCount);
					}

					TArray<FText> StoryLines = QuestManager->GetNPCStoryLines(AssociatedQuestID);
					if (StoryLines.Num() > 0)
					{
						UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
						if (Widget)
						{
							Widget->InitDialogueLines(AssociatedQuestID, DisplayName, StoryLines);
						}
					}

					Destroy();
				}
			}
		}
	}
}
