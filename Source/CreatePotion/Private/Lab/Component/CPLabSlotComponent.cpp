// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/CPLabSlotComponent.h"

#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Lab/CPLabPotionRequestTypes.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

namespace
{
	// PR 이후 제거 예정: 슬롯 상호작용 확인용 재료 이름 표시
	FString GetDebugIngredientName(const ACPAlchemyProp* IngredientProp)
	{
		if (!IsValid(IngredientProp)) return TEXT("없음");

		const UCPForageableItemData* ItemData = IngredientProp->GetSourceItemData();
		if (!ItemData) return IngredientProp->GetName();

		const FString DisplayName = ItemData->DisplayName.ToString();
		return DisplayName.IsEmpty() ? IngredientProp->GetName() : DisplayName;
	}
}

UCPLabSlotComponent::UCPLabSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SlotIndex = INDEX_NONE;
}

bool UCPLabSlotComponent::ExecuteInteraction(AActor* Interacter)
{
	if (!CanExecuteInteraction(Interacter)) return false;
	
	UCPLabPotionSessionComponent* Session = GetPotionSession();
	if (!Session) return false;

	ACPAlchemyProp* BeforeSlotProp = nullptr;
	Session->GetIngredientPropFromSlot(SlotIndex, BeforeSlotProp);
	ACPAlchemyProp* BeforeHeldProp = Session->GetHeldIngredientProp();

	const bool bInteracted = Session->InteractIngredientSlot(SlotIndex);

	ACPAlchemyProp* AfterSlotProp = nullptr;
	Session->GetIngredientPropFromSlot(SlotIndex, AfterSlotProp);
	ACPAlchemyProp* AfterHeldProp = Session->GetHeldIngredientProp();

	// PR 이후 제거 예정: DebugMessage에 Actor 인스턴스명 대신 DataAsset 표시 이름 출력
	const FString BeforeSlotName = GetDebugIngredientName(BeforeSlotProp);
	const FString BeforeHeldName = GetDebugIngredientName(BeforeHeldProp);
	const FString AfterSlotName = GetDebugIngredientName(AfterSlotProp);
	const FString AfterHeldName = GetDebugIngredientName(AfterHeldProp);

	// PR 이후 제거 예정: PR 확인을 위한 임시 DebugMessage
	if (GEngine){
		GEngine->AddOnScreenDebugMessage(
			-1,
			4.f,
			bInteracted ? FColor::Green : FColor::Red,
			FString::Printf(
				TEXT("슬롯 %d 상호작용: %s\n슬롯: %s -> %s\n손: %s -> %s"),
				SlotIndex,
				bInteracted ? TEXT("성공") : TEXT("실패"),
				*BeforeSlotName,
				*AfterSlotName,
				*BeforeHeldName,
				*AfterHeldName));
	}

	return bInteracted;
}

bool UCPLabSlotComponent::CanExecuteInteraction(AActor* Interacter) const
{
	if (!Super::CanExecuteInteraction(Interacter)) return false;
	
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
