// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "CPTrajectoryPreviewComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Carry), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPTrajectoryPreviewComponent : public USplineComponent
{
	GENERATED_BODY()
	
	public:
	UCPTrajectoryPreviewComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
