#include "NPC/CPAnimalNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"

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