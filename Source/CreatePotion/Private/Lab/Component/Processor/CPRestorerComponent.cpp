// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Processor/CPRestorerComponent.h"

#include "GameMode/CPLabGameMode.h"
#include "Lab/Actor/CPAlchemyProp.h"

bool UCPRestorerComponent::CanProcess(const ACPAlchemyProp* ItemInstance) const
{
	return IsValid(ItemInstance) && ItemInstance->GetSourceItemData() != nullptr;
}

void UCPRestorerComponent::ApplyProcess(ACPAlchemyProp* ItemInstance)
{
	if (!IsValid(ItemInstance)) return;
	
	if (ACPLabGameMode* LabGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ACPLabGameMode>() : nullptr){
		LabGameMode->RestoreUseLimit(ItemInstance);
	}
	
	ItemInstance->ResetToItemData();
}
