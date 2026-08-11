// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Lab/Component/CPLabInteractActionComponent.h"
#include "CPLabSlotComponent.generated.h"

class UCPLabPotionSessionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPLabSlotComponent : public UCPLabInteractActionComponent
{
	GENERATED_BODY()
	
public:
	UCPLabSlotComponent();
	
	virtual bool ExecuteInteraction(AActor* Interactor) override;
	virtual bool CanExecuteInteraction(AActor* Interactor) const override;
	
private:
	UCPLabPotionSessionComponent* GetPotionSession() const;
	
private:
	// 이 Slot이 담당하는 슬롯 번호
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lab|Slot", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;
};
