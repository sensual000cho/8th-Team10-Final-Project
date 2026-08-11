// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPLabInteractActionComponent.h"
#include "Components/ActorComponent.h"
#include "CPProcessorComponent.generated.h"

class ACPAlchemyProp;

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPProcessorComponent : public UCPLabInteractActionComponent
{
	GENERATED_BODY()

public:	
	UCPProcessorComponent();
	
	// 상호작용 시 호출(상속 받아 기능 구현)
	UFUNCTION(BlueprintCallable, Category = "Processor")
	bool ProcessItem(ACPAlchemyProp* ItemInstance);
	
	// 상호작용 시 재료를 가공
	virtual bool ExecuteInteraction(AActor* Interactor) override;
	// 현재 들고 있는 재료를 이 기구로 가공할 수 있는지 확인
	virtual bool CanExecuteInteraction(AActor* Interactor) const override;
	
	// 퀘스트 전환 시 초기화할 값이 있는 기구에서 구현(한 퀘스트 내에서 n회 제한)
	UFUNCTION(BlueprintCallable, Category = "Processor")
	virtual void ResetProcessor();
	
	// ItemInstance 때문에 소모된 사용 제한이 있다면 복구한다
	virtual bool RestoreUseLimit(const ACPAlchemyProp* ItemInstance);
	
protected:
	virtual bool CanProcess(const ACPAlchemyProp* ItemInstance) const;
	virtual void ApplyProcess(ACPAlchemyProp* ItemInstance);
	
	virtual bool NeedsResetRequestEnd() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Processor")
	FName ProcessorId;
		
};
