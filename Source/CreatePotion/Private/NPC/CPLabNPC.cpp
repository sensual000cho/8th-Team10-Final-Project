#include "NPC/CPLabNPC.h"
#include "Quest/QuestManager.h"
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"
#include "Settings/CPUISettings.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"

void ACPLabNPC::HandleInteract(AActor* Interactor)
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
	bWaitingForPotion = true;
	Super::HandleInteract(Interactor);
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) { return; }

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	if (!QuestManager || !UIManager) { return; }

	for (const FName& QuestID : QuestData->LabQuestIDs) {
		if (QuestID.IsNone()) continue;

		const FCPQuestResult QuestResult = QuestManager->GetResult(QuestID);
		if (!QuestResult.QuestId.IsNone()) {
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
			ActiveDialogueWidget->OnClosed.RemoveAll(this);
			ActiveDialogueWidget->OnClosed.AddUObject(this, &ACPLabNPC::OnDialogueClosed);

			RegisterCodex();
			FText NPCNameText = FText::FromName(QuestData->NPCName);
			TArray<FText> DialogueLines;
			DialogueLines.Add(FirstHint);

			ActiveDialogueWidget->InitDialogueLines(QuestID, NPCNameText, DialogueLines);
		}

		break;
	}
}

bool ACPLabNPC::CanInteract(AActor* Interactor) const
{
	if (!QuestData || QuestData->LabQuestIDs.IsEmpty() || !GetGameInstance()) { return false; }

	UQuestManager* QuestManager = GetGameInstance()->GetSubsystem<UQuestManager>();
	if (!QuestManager) { return false; }

	bool bCanQuestInteract = false;
	for (const FName& QuestID : QuestData->LabQuestIDs)
	{
		if (QuestID.IsNone()) { continue; }
		const FCPQuestResult QuestResult = QuestManager->GetResult(QuestID);
		if (!QuestResult.QuestId.IsNone())
		{
			bCanQuestInteract = true;
			break;
		}
		if (QuestManager->GetQuestState(QuestID) == EQuestState::Accepted)
		{
			bCanQuestInteract = true;
			break;
		}
	}

	if (bCanQuestInteract)
	{
		return Super::CanInteract(Interactor);
	}

	return false;
}

void ACPLabNPC::ShowResultDialogue()
{
	Super::ShowResultDialogue();

	if (ActiveDialogueWidget) {
		ActiveDialogueWidget->OnClosed.AddUObject(this, &ACPLabNPC::OnResultDialogueClosed);
	}
}

void ACPLabNPC::BeginPlay()
{
	Super::BeginPlay();
	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem())
	{
		Subsystem->SetNPCActiveStateById(GetPotionNPCId(), false, this);
	}
}

void ACPLabNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem())
	{
		Subsystem->SetNPCActiveStateById(GetPotionNPCId(), true, this);
	}

	Super::EndPlay(EndPlayReason);
}

const TArray<FName>& ACPLabNPC::GetPotionQuestIDs() const
{
	static const TArray<FName> QuestIDs;
	return QuestData ? QuestData->LabQuestIDs : QuestIDs;
}

void ACPLabNPC::OnTraceFocusLost()
{
	if (bWaitingForPotion)
	{
		bIsPlayerLooking = false;
		return;
	}
	Super::OnTraceFocusLost();
}

void ACPLabNPC::OnResultDialogueClosed()
{
	if (UWorld* World = GetWorld()) {
		if (ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(World->GetAuthGameMode())) {
			LabGameMode->ClearPotionRequest();
		}
	}
	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem())
	{
		Subsystem->SetNPCActiveStateById(GetPotionNPCId(), true, this);
	}
	Destroy();
}