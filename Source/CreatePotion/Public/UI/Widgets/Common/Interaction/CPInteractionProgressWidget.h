// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPInteractionProgressWidget.generated.h"

class UCPInteractionComponent;

// 상호작용 진행도 프로그레스바 위젯
UCLASS()
class CREATEPOTION_API UCPInteractionProgressWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetProgress(float InProgress);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ResetProgress();
	
protected:
	virtual void NativeConstruct() override;
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCPInteractionComponent> InteractionComponent;
};