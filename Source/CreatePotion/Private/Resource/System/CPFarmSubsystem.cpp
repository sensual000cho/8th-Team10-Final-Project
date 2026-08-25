#include "Resource/System/CPFarmSubsystem.h"
#include "Data/CPFarmCropDefinition.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"

void UCPFarmSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UCPTimeSubsystem* TimeSubsystem = Collection.InitializeDependency<UCPTimeSubsystem>();
	if (TimeSubsystem)
	{
		TimeSubsystem->OnTimeChanged.AddDynamic(this, &UCPFarmSubsystem::AdvanceFarmTime);
	}
}

void UCPFarmSubsystem::Deinitialize()
{
	UCPTimeSubsystem* TimeSubsystem = GetGameInstance()->GetSubsystem<UCPTimeSubsystem>();
	if (TimeSubsystem)
	{
		TimeSubsystem->OnTimeChanged.RemoveAll(this);
	}
	
	Super::Deinitialize();
}

bool UCPFarmSubsystem::RegisterTile(const FCPFarmTileKey& Key)
{
	if (FarmTileStates.Contains(Key))
	{
		return false;
	}
	
	FarmTileStates.Add(Key);
	
	return true;
}

const FCPFarmTileState* UCPFarmSubsystem::FindTileState(const FCPFarmTileKey& Key) const
{
	return FarmTileStates.Find(Key);
}

bool UCPFarmSubsystem::PlantCrop(const FCPFarmTileKey& Key, UCPFarmCropDefinition* CropDefinition)
{
	if (!CropDefinition) return false;
	
	FCPFarmTileState* TileState = FindMutableTileState(Key);
	if (!TileState) return false;
	if (!TileState->HasCrop()) return false;
	
	TileState->Crop.Initialize(CropDefinition);
	
	return true;
}

bool UCPFarmSubsystem::WaterTile(const FCPFarmTileKey& Key)
{
	FCPFarmTileState* TileState = FindMutableTileState(Key);
	
	if (!TileState) return false;
	
	TileState->SoilMoisture = MaxSoilMoisture;
	
	return true;
}

void UCPFarmSubsystem::AdvanceFarmTime(int64 DeltaGameMinutes)
{
	if (DeltaGameMinutes <= 0.f) return;
	
	const float MoistureDecayPerMinute = MaxSoilMoisture / FullMoistureDurationMinutes;
	
	for (auto& Pair : FarmTileStates)
	{
		FCPFarmTileState& TileState = Pair.Value;
		
		const float AvailableMoistureMinutes =
			TileState.SoilMoisture > 0.f ? TileState.SoilMoisture / MoistureDecayPerMinute : 0.f;
		
		const float ValidGrowthMinutes = FMath::Min(DeltaGameMinutes, AvailableMoistureMinutes);
		
		if (TileState.HasCrop() && !TileState.Crop.IsMature() && ValidGrowthMinutes > 0.f)
		{
			TileState.Crop.GrowthMinutes += ValidGrowthMinutes;
			TileState.Crop.GrowthMinutes =
				FMath::Min(TileState.Crop.GrowthMinutes, TileState.Crop.Definition->GrowthDuration);
		}
		
		TileState.SoilMoisture -= MoistureDecayPerMinute * DeltaGameMinutes;
		TileState.SoilMoisture = FMath::Clamp(TileState.SoilMoisture, 0.f, MaxSoilMoisture);
	}
}

FCPFarmTileState* UCPFarmSubsystem::FindMutableTileState(const FCPFarmTileKey& Key)
{
	return FarmTileStates.Find(Key);
}
