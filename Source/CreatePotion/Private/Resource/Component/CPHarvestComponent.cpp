#include "Resource/Component/CPHarvestComponent.h"

#include "Components/CPInventoryComponent.h"
#include "Data/CPResourceDefinition.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "Resource/System/CPObjectPoolSubsystem.h"
#include "Resource/System/CPResourceStateSubsystem.h"

UCPHarvestComponent::UCPHarvestComponent()
{
}

void UCPHarvestComponent::Initialize(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition,
                                     int32 InHarvestMultiplier)
{
	NodeKey = InNodeKey;
	ResourceDefinition = InDefinition;
	HarvestMultiplier = FMath::Max(1, InHarvestMultiplier);
	
	if (!IsValid(ResourceDefinition) || !IsValid(ResourceDefinition->HarvestData.HarvestedItem)){
		TargetName = NAME_None;
		InteractionPrompt = FText::GetEmpty();
		InteractionDuration = 0.f;
		RefreshUI();
		return;
	}
	
	TargetName = FName(*ResourceDefinition->HarvestData.HarvestedItem->DisplayName.ToString());
	InteractionDuration = ResourceDefinition->HarvestData.HarvestDuration;

	RefreshUI();
}

void UCPHarvestComponent::BeginFocus(AActor* Interactor)
{
	if (IsValid(ResourceDefinition) && IsValid(ResourceDefinition->HarvestData.HarvestedItem)){
		InteractionPrompt = FText::FromString(TEXT("채집하기"));
	}
	
	Super::BeginFocus(Interactor);
}

bool UCPHarvestComponent::CanInteract() const
{
	return IsValid(ResourceDefinition) 
		&& IsValid(ResourceDefinition->HarvestData.HarvestedItem) 
		&& ResourceDefinition->HarvestData.HarvestAmount * HarvestMultiplier > 0;
}

void UCPHarvestComponent::BeginTimeInteraction(AActor* Interactor)
{
	Super::BeginTimeInteraction(Interactor);
	
	if (!IsValid(Interactor) || !CanInteract()) return;
	
	USkeletalMeshComponent* CharacterMesh = Interactor->FindComponentByClass<USkeletalMeshComponent>();
	UAnimInstance* AnimInstance = IsValid(CharacterMesh) ? CharacterMesh->GetAnimInstance() : nullptr;
	if (!IsValid(AnimInstance) || !IsValid(HarvestMontage)) return;
	
	AnimInstance->Montage_Play(HarvestMontage);
}

bool UCPHarvestComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor) || !CanInteract() || !IsValid(Interactor)) return false;
	
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return false;
	
	UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (CodexSubsystem){
		CodexSubsystem->RecordForageableEntry(ResourceDefinition->HarvestData.HarvestedItem);
	}
	
	UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
	if (!Inventory) return false;
	
	const int32 Amount = ResourceDefinition->HarvestData.HarvestAmount * HarvestMultiplier;
	const int32 LeftOverCount = Inventory->TryGetItemFromData(ResourceDefinition->HarvestData.HarvestedItem, Amount);
	if (LeftOverCount != 0) return false;
	
	UCPResourceStateSubsystem* StateSubsystem = GI->GetSubsystem<UCPResourceStateSubsystem>();
	if (!StateSubsystem) return false;
	
	StateSubsystem->MarkHarvested(NodeKey, ResourceDefinition->RespawnDuration);
	UCPObjectPoolSubsystem* Pool = GetWorld() ? GetWorld()->GetSubsystem<UCPObjectPoolSubsystem>() : nullptr;
	if (Pool){
		Pool->ReleaseActor(GetOwner());
	}
	
	return true;
}