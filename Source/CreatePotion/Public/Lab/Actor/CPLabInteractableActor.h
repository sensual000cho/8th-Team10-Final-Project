// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "CPLabInteractableActor.generated.h"

class UCPLabInteractActionComponent;

UCLASS()
class CREATEPOTION_API ACPLabInteractableActor : public AActor, public ICPInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPLabInteractableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;	

public:
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual FName GetInteractionName_Implementation() override;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Lab|Interaction")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:	
	UPROPERTY(Transient)
	TObjectPtr<UCPLabInteractActionComponent> CachedActionComponent;
};
