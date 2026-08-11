// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Processor/CPCrusherComponent.h"

#include "Lab/Actor/CPAlchemyProp.h"

UCPCrusherComponent::UCPCrusherComponent(): ProcessMultiplier(2.f), MaxUseCount(1), UsedCount(0)
{
}

void UCPCrusherComponent::ResetProcessor()
{
	UsedCount = 0;
}

bool UCPCrusherComponent::CanProcess(const ACPAlchemyProp* ItemInstance) const
{
	return UsedCount < MaxUseCount && Super::CanProcess(ItemInstance);
}

void UCPCrusherComponent::ApplyProcess(ACPAlchemyProp* ItemInstance)
{
	if (!IsValid(ItemInstance)) return;
	
	ItemInstance->SetProcessMultiplier(ProcessMultiplier);
	UsedCount++;
}

EProcessorBlockReason UCPCrusherComponent::EvaluateIngredient(const ACPAlchemyProp* ItemInstance) const
{
	const EProcessorBlockReason BaseReason =
	Super::EvaluateIngredient(ItemInstance);

	if (BaseReason != EProcessorBlockReason::None)
	{
		return BaseReason;
	}

	if (UsedCount >= MaxUseCount)
	{
		return EProcessorBlockReason::SessionUseLimitReached;
	}

	return EProcessorBlockReason::None;
}

bool UCPCrusherComponent::NeedsResetRequestEnd() const
{
	return UsedCount > 0;
} 
