// Fill out your copyright notice in the Description page of Project Settings.


#include "CPBookMenuActor.h"


// Sets default values
ACPBookMenuActor::ACPBookMenuActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ACPBookMenuActor::SetMenuInteractable(bool bInteractable)
{
	if (UUserWidget* Menu = MenuWidgetComp->GetUserWidgetObject())
	{
		Menu->SetVisibility(bInteractable ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::HitTestInvisible);
	}
	
	MenuWidgetComp->SetCollisionEnabled(bInteractable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ACPBookMenuActor::BeginPlay()
{
	Super::BeginPlay();
	SetMenuInteractable(false);
	
}

// Called every frame
void ACPBookMenuActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

