#include "Items/Components/CPEggInteractableComponent.h"
#include "Components/CPInventoryComponent.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "Resource/System/CPObjectPoolSubsystem.h"

void UCPEggInteractableComponent::Initialize(UCPForageableItemData* InItemData, int32 InAmount)
{
	ItemData = InItemData;
	Amount = InAmount;
	TargetName = FName(ItemData->DisplayName.ToString());
}

bool UCPEggInteractableComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor) || !CanInteract() || !IsValid(Interactor)) return false;
	
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GI) return false;
	
	UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (CodexSubsystem)
	{
		CodexSubsystem->RecordForageableEntry(ItemData);
	}
	
	UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
	if (!Inventory) return false;
	
	const int32 LeftOverCount = Inventory->TryGetItemFromData(ItemData, Amount);
	if (LeftOverCount != 0) return false;
	
	UCPObjectPoolSubsystem* Pool = GetWorld() ? GetWorld()->GetSubsystem<UCPObjectPoolSubsystem>() : nullptr;
	if (Pool)
	{
		Pool->ReleaseActor(GetOwner());
	}
	
	return true;
}
