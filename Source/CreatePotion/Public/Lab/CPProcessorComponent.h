// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPProcessorComponent.generated.h"

class ACPAlchemyProp;

UCLASS( Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPProcessorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPProcessorComponent();
	
	// 상호작용 시 호출(상속 받아 기능 구현)
	UFUNCTION(BlueprintCallable, Category = "Processor")
	bool ProcessItem(ACPAlchemyProp* ItemInstance);
	
	// 퀘스트 전환 시 초기화할 값이 있는 기구에서 구현(한 퀘스트 내에서 n회 제한)
	UFUNCTION(BlueprintCallable, Category = "Processor")
	virtual void ResetProcessor();
	
protected:
	virtual bool CanProcess(const ACPAlchemyProp* ItemInstance) const;
	virtual void ApplyProcess(ACPAlchemyProp* ItemInstance);
	
protected:
	// Props에서 해당 값을 저장할 공간 필요(TSet 권장), 기구 해금 관리에도 사용(TSet 권장)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Processor")
	FName ProcessorId;
		
};
