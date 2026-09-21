#include "NPC/Components/CPNPCComponent.h"

#include "Data/NPC/CPNPCDataAsset.h"
#include "NPC/CPQuestNPC.h"

UCPNPCComponent::UCPNPCComponent()
{
	InteractionPrompt = FText::FromString(TEXT("대화하기"));
}

void UCPNPCComponent::BeginPlay()
{
	Super::BeginPlay();
	
	const ACPQuestNPC* OwnerNPC = Cast<ACPQuestNPC>(GetOwner());
	if (!IsValid(OwnerNPC)) return;
	
	const UCPNPCDataAsset* NPCData = OwnerNPC->GetNPCData();
	TargetName = NPCData && NPCData->NPCName != NAME_None ? NPCData->NPCName : OwnerNPC->GetFName();
}

void UCPNPCComponent::BeginFocus(AActor* Interactor)
{
	Super::BeginFocus(Interactor);
	
	ACPQuestNPC* OwnerNPC = Cast<ACPQuestNPC>(GetOwner());
	if (!IsValid(OwnerNPC)) return;
	
	OwnerNPC->SetFocused(true);
}

void UCPNPCComponent::EndFocus(AActor* Interactor)
{
	Super::EndFocus(Interactor);
	
	ACPQuestNPC* OwnerNPC = Cast<ACPQuestNPC>(GetOwner());
	if (!IsValid(OwnerNPC)) return;
	
	OwnerNPC->SetFocused(false);
}

bool UCPNPCComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor) || !IsValid(Interactor)) return false;

	ACPQuestNPC* OwnerNPC = Cast<ACPQuestNPC>(GetOwner());
	if (!IsValid(OwnerNPC)) return false;

	OwnerNPC->StartDialogue(Interactor);
	return true;
}