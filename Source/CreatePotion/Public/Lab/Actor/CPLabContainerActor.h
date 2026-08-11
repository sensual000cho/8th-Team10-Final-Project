// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPLabContainerActor.generated.h"

class UCPLabContainerComponent;

UCLASS()
class CREATEPOTION_API ACPLabContainerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPLabContainerActor();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container")
	UCPLabContainerComponent* LabContainerComponent;

};
