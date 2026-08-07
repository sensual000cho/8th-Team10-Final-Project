// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/CPProcessorComponent.h"

#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

// Sets default values for this component's properties
UCPProcessorComponent::UCPProcessorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCPProcessorComponent::ProcessItem(ACPAlchemyProp* ItemInstance)
{
	if (!CanProcess(ItemInstance)) return false;
	
	ApplyProcess(ItemInstance);
	// ItemInstance에 ProcessorId 등록(예정)
	return true;
}

bool UCPProcessorComponent::ExecuteInteraction(AActor* Interacter)
{
	if (!CanExecuteInteraction(Interacter)) return false;
	
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabGameState = World->GetGameState<ACPLabGameState>();
	UCPLabPotionSessionComponent* Session = LabGameState->GetPotionSession();
	
	return ProcessItem(Session->GetHeldIngredientProp());
}

bool UCPProcessorComponent::CanExecuteInteraction(AActor* Interacter) const
{
	if (!Super::CanExecuteInteraction(Interacter)) return false;
	
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;
	const UCPLabPotionSessionComponent* Session = LabGameState ? LabGameState->GetPotionSession() : nullptr;
	
	return Session && CanProcess(Session->GetHeldIngredientProp());
}

void UCPProcessorComponent::ResetProcessor()
{
	// 필요한 기구에서 상속하여 구현
}

bool UCPProcessorComponent::CanProcess(const ACPAlchemyProp* ItemInstance) const
{
	// ItemInstance가 ProcessorId가 등록되어 있는지 확인
	
	return ItemInstance && !ProcessorId.IsNone() && ItemInstance->GetSourceItemData() != nullptr;
}

void UCPProcessorComponent::ApplyProcess(ACPAlchemyProp* ItemInstance)
{
	// 상속한 각 기구에서 구현
}
