// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/CPProcessorComponent.h"

#include "GameMode/CPLabGameMode.h"
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
	
	if (!ProcessorId.IsNone()){
		ItemInstance->MarkProcessedBy(ProcessorId);
	}
	
	if (NeedsResetRequestEnd()){
		ACPLabGameMode* LabGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ACPLabGameMode>() : nullptr;
		if (LabGameMode){
			LabGameMode->RegisterProcessor(this);
		}
	}

	return true;
}

bool UCPProcessorComponent::ExecuteInteraction(AActor* Interactor)
{
	if (!CanExecuteInteraction(Interactor)) return false;
	
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabGameState = World->GetGameState<ACPLabGameState>();
	UCPLabPotionSessionComponent* Session = LabGameState->GetPotionSession();
	
	return ProcessItem(Session->GetHeldIngredientProp());
}

bool UCPProcessorComponent::CanExecuteInteraction(AActor* Interactor) const
{
	if (!Super::CanExecuteInteraction(Interactor)) return false;
	
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;
	const UCPLabPotionSessionComponent* Session = LabGameState ? LabGameState->GetPotionSession() : nullptr;
	if (!Session) return false;
	
	FCPLabPotionRequestState ActiveRequestState;
	if (!Session->GetActiveRequestState(ActiveRequestState) || 
		ActiveRequestState.Phase != ECPLabPotionRequestPhase::Processing) return false;
	
	return CanProcess(Session->GetHeldIngredientProp());
}

void UCPProcessorComponent::ResetProcessor()
{
	// 필요한 기구에서 상속하여 구현
}

bool UCPProcessorComponent::RestoreUseLimit(const ACPAlchemyProp* ItemInstance)
{
	// Prop 단위 사용 제한을 관리하는 기구에서만 구현
	return false;
}

bool UCPProcessorComponent::TryBuildPreviewEffects(
	const ACPAlchemyProp* ItemInstance,
	TMap<FGameplayTag, int32>& OutPreviewEffects) const
{
	OutPreviewEffects.Reset();

	if (!CanProcess(ItemInstance)) return false;

	const FCPLabIngredientInstance Ingredient = ItemInstance->GetWorkingIngredient();
	if (!Ingredient.IsValid()) return false;

	OutPreviewEffects = Ingredient.CurrentEffects;
	return BuildPreviewEffects(ItemInstance, OutPreviewEffects);
}

bool UCPProcessorComponent::CanProcess(const ACPAlchemyProp* ItemInstance) const
{
	if (!IsValid(ItemInstance) || ItemInstance->GetSourceItemData() == nullptr) return false;
	
	return ProcessorId.IsNone() || !ItemInstance->HasBeenProcessedBy(ProcessorId);
}

void UCPProcessorComponent::ApplyProcess(ACPAlchemyProp* ItemInstance)
{
	// 상속한 각 기구에서 구현
}

bool UCPProcessorComponent::BuildPreviewEffects(
	const ACPAlchemyProp* ItemInstance,
	TMap<FGameplayTag, int32>& InOutPreviewEffects) const
{
	return false;
}

bool UCPProcessorComponent::NeedsResetRequestEnd() const
{
	return false;
}
