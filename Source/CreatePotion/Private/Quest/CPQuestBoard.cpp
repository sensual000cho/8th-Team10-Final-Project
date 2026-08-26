// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/CPQuestBoard.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"

ACPQuestBoard::ACPQuestBoard()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACPQuestBoard::OnInteract_Implementation(AActor* Interactor)
{
	if (!QuestBoardWidgetClass) return;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>())
		{
			UIManager->PushWidget(QuestBoardWidgetClass);
		}
	}
}

bool ACPQuestBoard::CanInteract_Implementation(AActor* Interactor)
{
	return true;
}

FText ACPQuestBoard::GetInteractionPrompt_Implementation()
{
	if (!CanInteract_Implementation(nullptr))
	{
		return FText::GetEmpty();
	}

	return FText::FromString(TEXT("F : 의뢰 확인하기"));
}

