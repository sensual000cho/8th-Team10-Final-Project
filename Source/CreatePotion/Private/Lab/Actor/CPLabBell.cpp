// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Actor/CPLabBell.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameMode/CPLabGameMode.h"
#include "GameState/CPLabGameState.h"
#include "Lab/CPLabTypes.h"


ACPLabBell::ACPLabBell()
{
	PrimaryActorTick.bCanEverTick = false;
	BellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BellMesh"));
	SetRootComponent(BellMesh);
	BellMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BellMesh->SetCollisionObjectType(ECC_WorldDynamic);
	//일단 Ignore, 필요한 것만 반응
	BellMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BellMesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	BellMesh->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
}


void ACPLabBell::OnInteract_Implementation(AActor* Interactor)
{
	TryRingBell();
}

FText ACPLabBell::GetInteractionPrompt_Implementation()
{
	return FText::FromString(TEXT("개점하기"));
}

bool ACPLabBell::CanInteract_Implementation(AActor* Interactor)
{
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabState =
		World ? World->GetGameState<ACPLabGameState>() : nullptr;

	return LabState &&
		LabState->GetCurrentPhase() ==
			ECPLabSessionPhase::WaitingForBell;
}

bool ACPLabBell::TryRingBell()
{
	UWorld* World = GetWorld();
	ACPLabGameMode* LabMode =
		World
			? World->GetAuthGameMode<ACPLabGameMode>()
			: nullptr;

	return LabMode && LabMode->TryStartLabSession();
}


