// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab/Component/CPProcessorComponent.h"
#include "CPCrusherComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPCrusherComponent : public UCPProcessorComponent
{
	GENERATED_BODY()
	
public:
	UCPCrusherComponent();
	
	// Prop 단위 사용 제한 소모 상태를 초기화 한다
	virtual void ResetProcessor() override;
	
	// 해당 Prop이 소모한 분쇄기 사용 제한을 복구한다
	virtual bool RestoreUseLimit(const ACPAlchemyProp* ItemInstance) override;
	
protected:
	virtual bool CanProcess(const ACPAlchemyProp* ItemInstance) const override;
	virtual void ApplyProcess(ACPAlchemyProp* ItemInstance) override;
	
	// 초기화 대상 등록이 필요한지 확인한다
	virtual bool NeedsResetRequestEnd() const override;
	
private:
	// 현재 유효한 사용 제한을 확인한다
	int32 GetProcessedPropCount() const;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Lab/Processor")
	float ProcessMultiplier;
	
	UPROPERTY(EditDefaultsOnly, Category = "Lab/Processor")
	int32 MaxUseCount;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Lab/Processor")
	TArray<TWeakObjectPtr<ACPAlchemyProp>> ProcessedProps;
};
