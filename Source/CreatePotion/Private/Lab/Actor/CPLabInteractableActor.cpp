// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Actor/CPLabInteractableActor.h"

#include "Lab/Component/CPLabInteractActionComponent.h"

// Sets default values
ACPLabInteractableActor::ACPLabInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

// Called when the game starts or when spawned
void ACPLabInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
	CachedActionComponent = FindComponentByClass<UCPLabInteractActionComponent>();
}

void ACPLabInteractableActor::OnInteract_Implementation(AActor* Interactor)
{
	if (CachedActionComponent){
		CachedActionComponent->ExecuteInteraction(Interactor);
	}
}

bool ACPLabInteractableActor::CanInteract_Implementation(AActor* Interactor)
{
	return CachedActionComponent && CachedActionComponent->CanExecuteInteraction(Interactor);
}

FText ACPLabInteractableActor::GetInteractionPrompt_Implementation()
{
	return CachedActionComponent ? CachedActionComponent->GetInteractionPrompt() : FText::GetEmpty();
}
