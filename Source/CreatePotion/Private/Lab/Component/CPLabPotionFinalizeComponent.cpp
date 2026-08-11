// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/CPLabPotionFinalizeComponent.h"

#include "GameMode/CPLabGameMode.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"


bool UCPLabPotionFinalizeComponent::ExecuteInteraction(AActor* Interactor)
{
	if (!CanExecuteInteraction(Interactor)) return false;
	
	UWorld* World = GetWorld();
	ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;
	if (!LabGameMode) return false;
	
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;
	
	return LabGameMode->FinalizePotionAtActor(OwnerActor);
}

bool UCPLabPotionFinalizeComponent::CanExecuteInteraction(AActor* Interactor) const
{
	if (!Super::CanExecuteInteraction(Interactor)) return false;
	
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;
	const UCPLabPotionSessionComponent* Session = LabGameState ? LabGameState->GetPotionSession() : nullptr;
	if (!Session) return false;
	
	FCPLabPotionRequestState ActiveRequestState;
	return Session->GetActiveRequestState(ActiveRequestState) && 
		ActiveRequestState.Phase == ECPLabPotionRequestPhase::Processing;
}
