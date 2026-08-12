// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPLabNPC.generated.h"

class UCPNPCDialogueWidget;
class UCPLabResultWidget;

UCLASS()
class CREATEPOTION_API ACPLabNPC : public ACPBaseNPC
{
	GENERATED_BODY()
public:
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;

	void SetRequestConfirmed(bool bConfirmed) { bRequestConfirmed = bConfirmed; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCPNPCDialogueWidget> DialogueWidgetClass;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCPLabResultWidget> ResultWidgetClass;
private:
	bool bRequestConfirmed = false;
	
	//Result UI 결과 송출용 함수들
	UFUNCTION()
	void HandleResultAccepted();

	UFUNCTION()
	void HandleResultRetryRequested();

	UPROPERTY(Transient)
	TObjectPtr<UCPLabResultWidget> ActiveResultWidget;

};
