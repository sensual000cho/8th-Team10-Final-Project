#include "NPC/CPAnimalNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"

ACPAnimalNPC::ACPAnimalNPC()
{
}

FName ACPAnimalNPC::GetPotionNPCId() const
{
	if (UniqueAnimalID != NAME_None)
	{
		return UniqueAnimalID;
	}
	return GetFName();
}