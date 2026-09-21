#include "NPC/CPAnimalNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "Quest/QuestManager.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Lab/CPLabResultWidget.h"

ACPAnimalNPC::ACPAnimalNPC()
{
}

FName ACPAnimalNPC::GetPotionNPCId() const
{
	if (!CachedPotionNPCId.IsNone())
	{
		return CachedPotionNPCId;
	}

	const FString UniquePathName = GetPathName();
	CachedPotionNPCId = FName(*UniquePathName);

	return CachedPotionNPCId;
}

void ACPAnimalNPC::HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags)
{
	if (AssociatedQuestID.IsNone()) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>();
	if (!QuestManager) return;

	QuestManager->CreateResult(AssociatedQuestID, PotionEffectTags);
	ShowResultWidget(AssociatedQuestID);
}

void ACPAnimalNPC::ShowResultWidget(FName QuestID)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* ResultWidgetClass = UISettings ? UISettings->ResultWidgetClass.LoadSynchronous() : nullptr;

	if (UIManager && ResultWidgetClass)
	{
		UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
		if (ResultWidget && !ResultWidget->InitializeResult(QuestID))
		{
			ResultWidget->RequestClose();
		}
	}
}