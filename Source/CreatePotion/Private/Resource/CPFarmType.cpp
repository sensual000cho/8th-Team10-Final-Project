#include "Resource/CPFarmType.h"
#include "Data/CPFarmCropDefinition.h"

void FCPFarmCropInstance::Initialize(UCPFarmCropDefinition* InDefinition)
{
	Definition = InDefinition;
	GrowthMinutes = 0.f;	
}

bool FCPFarmCropInstance::IsValid() const
{
	return Definition != nullptr;
}

void FCPFarmCropInstance::Reset()
{
	Definition = nullptr;
	GrowthMinutes = 0;
}

float FCPFarmCropInstance::GetGrowthRatio() const
{
	if (!Definition || Definition->GrowthDuration <= 0)
	{
		return 0.f;
	}
	
	return FMath::Clamp(GrowthMinutes / Definition->GrowthDuration, 0.f, 1.f);
}

ECPCropGrowthStage FCPFarmCropInstance::GetGrowthStage() const
{
	if (!Definition)
	{
		return ECPCropGrowthStage::None;
	}
	
	const float GrowthRatio = GetGrowthRatio();
	
	if (GrowthRatio >= 1.f)
	{
		return ECPCropGrowthStage::Mature;
	}
	
	if (GrowthRatio >= Definition->IntermediateStageRatio)
	{
		return ECPCropGrowthStage::Intermediate;
	}
	
	return ECPCropGrowthStage::Seed;
}

bool FCPFarmCropInstance::IsMature() const
{
	return IsValid() && GrowthMinutes >= Definition->GrowthDuration;
}

bool FCPFarmTileState::HasCrop() const
{
	return Crop.IsValid();
}

bool FCPFarmTileKey::operator==(const FCPFarmTileKey& Other) const
{
	return FarmId == Other.FarmId && Coordinate == Other.Coordinate;
}

uint32 GetTypeHash(const FCPFarmTileKey& Key)
{
	return HashCombine(GetTypeHash(Key.FarmId), GetTypeHash(Key.Coordinate));
}
