#include "Quest/Components/CPQuestBoardComponent.h"

#include "Components/CPInventoryComponent.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"

UCPQuestBoardComponent::UCPQuestBoardComponent()
{
	InteractionPrompt = FText::FromString(TEXT("F : 의뢰 확인하기"));
	TargetName = FName(TEXT("QuestBoard"));
}

bool UCPQuestBoardComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor)) return false;

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GameInstance) return false;

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) return false;

	UCPInventoryComponent* Inventory = IsValid(Interactor)
		? Interactor->FindComponentByClass<UCPInventoryComponent>()
		: nullptr;

	if (Inventory)
	{
		const TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();

		for (const FName& QuestID : QuestManager->GetTrackedRandomQuestIDs())
		{
			if (QuestManager->GetQuestState(QuestID) != EQuestState::Accepted) continue;
			if (QuestManager->GetQuestCompletionType(QuestID) != EQuestCompletionType::ItemCollection) continue;

			if (QuestManager->TryCompleteQuestByItem(QuestID, HeldItems))
			{
				TArray<FName> RequiredIDs;
				int32 RequiredCount = 0;
				QuestManager->GetRequiredItemInfo(QuestID, RequiredIDs, RequiredCount);

				for (const FName& ItemID : RequiredIDs)
				{
					Inventory->RemoveItemByID(ItemID, RequiredCount);
				}

				QuestManager->CreateResult(QuestID);

				const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
				UClass* ResultWidgetClass = UISettings ? UISettings->ResultWidgetClass.LoadSynchronous() : nullptr;
				if (UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>())
				{
					if (ResultWidgetClass)
					{
						UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
						if (ResultWidget && !ResultWidget->InitializeResult(QuestID))
						{
							ResultWidget->RequestClose();
						}
					}
				}

				UE_LOG(LogTemp, Log, TEXT("[게시판] 랜덤 퀘스트 [%s] 완료 처리됨"), *QuestID.ToString());
			}
		}
	}

	if (!QuestBoardWidgetClass) return true;

	if (UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>())
	{
		UIManager->PushWidget(QuestBoardWidgetClass);
	}

	return true;
}
