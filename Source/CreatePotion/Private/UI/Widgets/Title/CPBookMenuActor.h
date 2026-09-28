// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/BookMenuActor.h"

#include "CPBookMenuActor.generated.h"

UCLASS()
class CREATEPOTION_API ACPBookMenuActor : public ABookMenuActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACPBookMenuActor();

	void SetMenuInteractable(bool bInteractable);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWidgetComponent> MenuWidgetComp;
};
