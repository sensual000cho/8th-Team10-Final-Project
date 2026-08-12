// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/CPProcessorComponent.h"

#include "Data/CPForageableItemData.h"
#include "GameMode/CPLabGameMode.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

namespace
{
	// PR 이후 제거 예정: 가공 전후 태그 값 확인용 재료 이름 표시
	FString GetProcessorDebugIngredientName(const ACPAlchemyProp* IngredientProp)
	{
		if (!IsValid(IngredientProp)) return TEXT("없음");

		const UCPForageableItemData* ItemData = IngredientProp->GetSourceItemData();
		if (!ItemData) return IngredientProp->GetName();

		const FString DisplayName = ItemData->DisplayName.ToString();
		return DisplayName.IsEmpty() ? IngredientProp->GetName() : DisplayName;
	}

	// PR 이후 제거 예정: 가공 전후 태그 값 확인용 문자열 생성
	FString GetDebugEffectText(const FCPLabIngredientInstance& Ingredient)
	{
		if (!Ingredient.IsValid() || Ingredient.CurrentEffects.IsEmpty()) return TEXT("없음");

		FString Result;
		for (const TPair<FGameplayTag, int32>& Effect : Ingredient.CurrentEffects){
			if (!Effect.Key.IsValid()) continue;

			if (!Result.IsEmpty()){
				Result += TEXT(", ");
			}

			Result += FString::Printf(
				TEXT("%s:%d"),
				*Effect.Key.ToString(),
				Effect.Value);
		}

		return Result.IsEmpty() ? TEXT("없음") : Result;
	}
}

// Sets default values for this component's properties
UCPProcessorComponent::UCPProcessorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCPProcessorComponent::ProcessItem(ACPAlchemyProp* ItemInstance)
{
	if (!CanProcess(ItemInstance)) return false;

	const FCPLabIngredientInstance BeforeIngredient = ItemInstance->GetWorkingIngredient();
	
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

	const FCPLabIngredientInstance AfterIngredient = ItemInstance->GetWorkingIngredient();

	// PR 이후 제거 예정: 가공 전후 태그 값 확인용 임시 DebugMessage
	if (GEngine){
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Cyan,
			FString::Printf(
				TEXT("가공기구: %s\n재료: %s\n가공 전: %s\n가공 후: %s"),
				*ProcessorId.ToString(),
				*GetProcessorDebugIngredientName(ItemInstance),
				*GetDebugEffectText(BeforeIngredient),
				*GetDebugEffectText(AfterIngredient)));
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
