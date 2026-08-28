// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPLabInteractActionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPLabInteractActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCPLabInteractActionComponent();
	
	virtual bool ExecuteInteraction(AActor* Interactor);
	virtual bool CanExecuteInteraction(AActor* Interactor) const;
	virtual FText GetInteractionPrompt() const;
	virtual FName GetInteractionName() const;

	virtual bool ShouldShowUnavailableInteraction(AActor* Interactor) const;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lab|Interaction")
	FText InteractionPrompt;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lab|Interaction")
	FName InteractionName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lab|Interaction")
	bool bEnabled;
		
	// 상호작용 불가능일때 회색 UI 표시를 결정하는 bool 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lab|Interaction")
	bool bShowWhenUnavailable;
};
