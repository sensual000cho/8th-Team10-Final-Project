// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CPTrajectoryPreviewComponent.h"

UCPTrajectoryPreviewComponent::UCPTrajectoryPreviewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCastShadow(false);
	SetHiddenInGame(true);
	SetVisibility(false);
}

void UCPTrajectoryPreviewComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCPTrajectoryPreviewComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UCPTrajectoryPreviewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}