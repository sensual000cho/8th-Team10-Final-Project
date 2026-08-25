#include "Resource/Actor/CPFarmTileActor.h"
#include "Data/CPFarmCropDefinition.h"
#include "Resource/System/CPFarmSubsystem.h"

ACPFarmTileActor::ACPFarmTileActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACPFarmTileActor::OnInteract_Implementation(AActor* Interactor)
{
	ICPInteractable::OnInteract_Implementation(Interactor);
}

FText ACPFarmTileActor::GetInteractionPrompt_Implementation()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return FText::GetEmpty();
	
	UCPFarmSubsystem* FarmSubsystem = GI->GetSubsystem<UCPFarmSubsystem>();
	if (!FarmSubsystem) return FText::GetEmpty();
	
	const FCPFarmTileState* State = FarmSubsystem->FindTileState(GetTileKey());
	if (!State) return FText::GetEmpty();
	
	if (!State->HasCrop())
	{
		return FText::FromString(TEXT("심기"));
	}
	
	// TODO: 물주기
	
	if (State->Crop.IsMature())
	{
		return FText::FromString(TEXT("수확하기"));
	}
	
	return FText::GetEmpty();
}

FName ACPFarmTileActor::GetInteractionName_Implementation()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return NAME_None;
	
	UCPFarmSubsystem* FarmSubsystem = GI->GetSubsystem<UCPFarmSubsystem>();
	if (!FarmSubsystem) return NAME_None;
	
	const FCPFarmTileState* State = FarmSubsystem->FindTileState(GetTileKey());
	if (!State) return NAME_None;
	
	return FName(*State->Crop.Definition->HarvestData.HarvestedItem->DisplayName.ToString());
}

bool ACPFarmTileActor::CanInteract_Implementation(AActor* Interactor)
{
	return ICPInteractable::CanInteract_Implementation(Interactor);
}

float ACPFarmTileActor::GetInteractionDuration_Implementation(AActor* Interactor)
{
	return ICPTimedInteractable::GetInteractionDuration_Implementation(Interactor);
}

void ACPFarmTileActor::BeginPlay()
{
	Super::BeginPlay();
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPFarmSubsystem* FarmSubsystem = GI->GetSubsystem<UCPFarmSubsystem>();
	if (!FarmSubsystem) return;
	
	FarmSubsystem->RegisterTile(GetTileKey());
	
	RefreshVisual();
}

FCPFarmTileKey ACPFarmTileActor::GetTileKey() const
{
	FCPFarmTileKey Key;
	Key.FarmId = FarmId;
	Key.Coordinate = GridCoordinate;
	
	return Key;
}

void ACPFarmTileActor::RefreshVisual()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPFarmSubsystem* FarmSubsystem = GI->GetSubsystem<UCPFarmSubsystem>();
	if (!FarmSubsystem) return;
	
	const FCPFarmTileState* TileState = FarmSubsystem->FindTileState(GetTileKey());
	if (!TileState) return;
	
	if (!TileState->HasCrop())
	{
		CropMesh->SetStaticMesh(nullptr);
		return;
	}
	
	const FCPFarmCropInstance& Crop = TileState->Crop;
	ECPCropGrowthStage Stage = Crop.GetGrowthStage();
	
	if (Stage == ECPCropGrowthStage::Seed)
	{
		CropMesh->SetStaticMesh(Crop.Definition->SeedMesh.LoadSynchronous());
	}
	
	else if (Stage == ECPCropGrowthStage::Intermediate)
	{
		CropMesh->SetStaticMesh(Crop.Definition->IntermediateMesh.LoadSynchronous());
	}
	
	else if (Stage == ECPCropGrowthStage::Mature)
	{
		CropMesh->SetStaticMesh(Crop.Definition->MatureMesh.LoadSynchronous());
	}
}

