// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/CPStoryActor.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "Components/CPInteractableComponent.h"

ACPStoryActor::ACPStoryActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultRoot;

	InteractableComponent = CreateDefaultSubobject<UCPInteractableComponent>(TEXT("InteractableComponent"));
}

void ACPStoryActor::BeginPlay()
{
	Super::BeginPlay();

	if (InteractableComponent)
	{
		InteractableComponent->SetInteractability(true);
		InteractableComponent->SetInteractionPrompt(FText::FromString(TEXT("F : 읽기")));
		InteractableComponent->SetTargetName(FName("StoryText"));
		InteractableComponent->OnActorInteractionCompleted.AddUniqueDynamic(this, &ACPStoryActor::HandleInteract);
	}
}

void ACPStoryActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InteractableComponent)
	{
		InteractableComponent->OnActorInteractionCompleted.RemoveDynamic(this, &ACPStoryActor::HandleInteract);
	}

	Super::EndPlay(EndPlayReason);
}

void ACPStoryActor::HandleInteract(AActor* Interactor)
{
	if (!DialogueWidgetClass || LoreLines.Num() == 0) return;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>())
		{
			if (UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass)))
			{
				Widget->InitDialogueLines(NAME_None, DisplayName, LoreLines);
			}
		}
	}
}
