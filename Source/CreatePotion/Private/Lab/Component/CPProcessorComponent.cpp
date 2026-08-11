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
	ItemInstance->MarkProcessedBy(ProcessorId);
	
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
	
	// const UWorld* World = GetWorld();
	// const ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;
	// const UCPLabPotionSessionComponent* Session = LabGameState ? LabGameState->GetPotionSession() : nullptr;
	// if (!Session) return false;
	//
	// FCPLabPotionRequestState ActiveRequestState;
	// if (!Session->GetActiveRequestState(ActiveRequestState) || 
	// 	ActiveRequestState.Phase != ECPLabPotionRequestPhase::Processing) return false;
	//
	// return CanProcess(Session->GetHeldIngredientProp());
	return EvaluateInteraction() ==	EProcessorBlockReason::None;
}

FText UCPProcessorComponent::GetInteractionPrompt() const
{
	switch (EvaluateInteraction())
	{
	case EProcessorBlockReason::None:
		return Super::GetInteractionPrompt();

	case EProcessorBlockReason::Disabled:
		return FText::FromString(
			TEXT("사용 불가: 현재 사용할 수 없습니다"));

	case EProcessorBlockReason::NotProcessingPhase:
		return FText::FromString(
			TEXT("사용 불가: 지금은 가공 단계가 아닙니다"));

	case EProcessorBlockReason::NoHeldIngredient:
		return FText::FromString(
			TEXT("사용 불가: 가공할 재료를 들어 주세요"));

	case EProcessorBlockReason::InvalidIngredient:
		return FText::FromString(
			TEXT("사용 불가: 이 재료는 가공할 수 없습니다"));

	case EProcessorBlockReason::AlreadyApplied:
		return FText::FromString(
			TEXT("사용 불가: 이미 이 재료에 사용했습니다"));

	case EProcessorBlockReason::SessionUseLimitReached:
		return FText::FromString(
			TEXT("사용 불가: 이번 제조에서 이미 사용했습니다"));

	default:
		return FText::FromString(
			TEXT("사용할 수 없습니다"));
	}
}

void UCPProcessorComponent::ResetProcessor()
{
	// 필요한 기구에서 상속하여 구현
}

bool UCPProcessorComponent::CanProcess(const ACPAlchemyProp* ItemInstance) const
{
	// if (!IsValid(ItemInstance) || ProcessorId.IsNone()) return false;
	//
	// return ItemInstance->GetSourceItemData() != nullptr && !ItemInstance->HasBeenProcessedBy(ProcessorId);
	
	return EvaluateIngredient(ItemInstance) ==	EProcessorBlockReason::None;
}

void UCPProcessorComponent::ApplyProcess(ACPAlchemyProp* ItemInstance)
{
	// 상속한 각 기구에서 구현
}

bool UCPProcessorComponent::NeedsResetRequestEnd() const
{
	return false;
}

EProcessorBlockReason UCPProcessorComponent::EvaluateInteraction() const
{
	if (!bEnabled)
	{
		return EProcessorBlockReason::Disabled;
	}

	const UWorld* World = GetWorld();
	const ACPLabGameState* LabGameState =
		World ? World->GetGameState<ACPLabGameState>() : nullptr;

	const UCPLabPotionSessionComponent* Session =
		LabGameState ? LabGameState->GetPotionSession() : nullptr;

	if (!Session)
	{
		return EProcessorBlockReason::NotProcessingPhase;
	}

	FCPLabPotionRequestState ActiveRequestState;
	if (!Session->GetActiveRequestState(ActiveRequestState) ||
		ActiveRequestState.Phase !=
			ECPLabPotionRequestPhase::Processing)
	{
		return EProcessorBlockReason::NotProcessingPhase;
	}

	const ACPAlchemyProp* HeldIngredient =
		Session->GetHeldIngredientProp();

	if (!IsValid(HeldIngredient))
	{
		return EProcessorBlockReason::NoHeldIngredient;
	}

	const EProcessorBlockReason IngredientReason =
		EvaluateIngredient(HeldIngredient);

	if (IngredientReason != EProcessorBlockReason::None)
	{
		return IngredientReason;
	}

	// 건조기·증류기 자식 클래스의 추가 CanProcess 조건 확인
	if (!CanProcess(HeldIngredient))
	{
		return EProcessorBlockReason::InvalidIngredient;
	}

	return EProcessorBlockReason::None;
}

EProcessorBlockReason UCPProcessorComponent::EvaluateIngredient(const ACPAlchemyProp* ItemInstance) const
{
	if (!IsValid(ItemInstance) || ProcessorId.IsNone() || ItemInstance->GetSourceItemData() == nullptr)
	{
		return EProcessorBlockReason::InvalidIngredient;
	}

	if (ItemInstance->HasBeenProcessedBy(ProcessorId))
	{
		return EProcessorBlockReason::AlreadyApplied;
	}

	return EProcessorBlockReason::None;
}
