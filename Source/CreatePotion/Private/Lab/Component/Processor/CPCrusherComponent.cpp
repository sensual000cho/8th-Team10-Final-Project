// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Processor/CPCrusherComponent.h"

#include "Lab/Actor/CPAlchemyProp.h"

UCPCrusherComponent::UCPCrusherComponent(): ProcessMultiplier(2.f), MaxUseCount(1)
{
}

void UCPCrusherComponent::ResetProcessor()
{
	ProcessedProps.Reset();
}

bool UCPCrusherComponent::RestoreUseLimit(const ACPAlchemyProp* ItemInstance)
{
	if (!IsValid(ItemInstance)) return false;
	
	bool bRestoredUseLimit = false;
	
	for (int32 Index = ProcessedProps.Num() - 1; Index >= 0; --Index){
		const TWeakObjectPtr<ACPAlchemyProp>& ProcessedProp = ProcessedProps[Index];
		
		// 삭제된 Prop은 정리
		if (!ProcessedProp.IsValid()){
			ProcessedProps.RemoveAtSwap(Index);
			continue;
		}
		
		// 대상 Prop이 소모했던 분쇄기 사용 제한을 복구한다
		if (ProcessedProp.Get() == ItemInstance){
			ProcessedProps.RemoveAtSwap(Index);
			bRestoredUseLimit = true;
		}
	}
	
	return bRestoredUseLimit;
}

bool UCPCrusherComponent::CanProcess(const ACPAlchemyProp* ItemInstance) const
{
	// 리퀘스트 내 사용 제한 소모 수 + processorId로 확인
	return GetProcessedPropCount() < MaxUseCount && Super::CanProcess(ItemInstance);
}

void UCPCrusherComponent::ApplyProcess(ACPAlchemyProp* ItemInstance)
{
	if (!IsValid(ItemInstance)) return;
	
	ItemInstance->SetProcessMultiplier(ProcessMultiplier);
	// 분쇄기 사용 제한을 소모한 prop을 기록
	ProcessedProps.Add(ItemInstance);
}

bool UCPCrusherComponent::NeedsResetRequestEnd() const
{
	// 사용제한을 소모한 prop이 있는 경우
	return GetProcessedPropCount() > 0;
}

int32 UCPCrusherComponent::GetProcessedPropCount() const
{
	int32 Count = 0;
	
	// Destroy된 prop 참조을 제외하고 count
	for(const TWeakObjectPtr<ACPAlchemyProp>& ProcessedProp : ProcessedProps){
		if (ProcessedProp.IsValid()) ++Count;
	}
	
	return Count;
} 
