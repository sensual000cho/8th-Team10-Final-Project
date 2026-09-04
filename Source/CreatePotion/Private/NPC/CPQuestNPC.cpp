#include "NPC/CPQuestNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"       
#include "Data/NPC/CPQuestNPCDataAsset.h"

ACPQuestNPC::ACPQuestNPC()
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

const UCPQuestNPCDataAsset* ACPQuestNPC::GetQuestNPCData() const
{
	return Cast<UCPQuestNPCDataAsset>(NPCData);
}
