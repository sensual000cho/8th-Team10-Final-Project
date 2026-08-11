// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/CPLabSlotComponent.h"

#include "GameState/CPLabGameState.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

UCPLabSlotComponent::UCPLabSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SlotIndex = INDEX_NONE;
}

bool UCPLabSlotComponent::ExecuteInteraction(AActor* Interactor)
{
	if (!CanExecuteInteraction(Interactor)) return false;
	
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;

	return Session->InteractIngredientSlot(SlotIndex);
}

bool UCPLabSlotComponent::CanExecuteInteraction(AActor* Interactor) const
{
	if (!Super::CanExecuteInteraction(Interactor)) return false;
	
	const UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session || SlotIndex == INDEX_NONE) return false;
	
	FCPLabPotionRequestState ActiveRequestState;
	if (!Session->GetActiveRequestState(ActiveRequestState)) return false;
	if (ActiveRequestState.Phase != ECPLabPotionRequestPhase::Preparing && 
			ActiveRequestState.Phase != ECPLabPotionRequestPhase::Processing) return false;
	
	ACPAlchemyProp* SlotIngredientProp = nullptr;
	const bool bHasSlotIngredient = Session->GetIngredientPropFromSlot(SlotIndex, SlotIngredientProp);
	
	return bHasSlotIngredient || Session->HasHeldIngredient();
}

UCPLabPotionSessionComponent* UCPLabSlotComponent::GetPotionSession() const
{
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;
	
	return LabGameState ? LabGameState->GetPotionSession() : nullptr;
}
