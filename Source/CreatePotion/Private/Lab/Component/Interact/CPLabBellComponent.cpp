// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPLabBellComponent.h"

#include "Character/CPInteractionComponent.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Quest/CPQuestSelectWidget.h"

UCPLabBellComponent::UCPLabBellComponent()
{
	InteractionPrompt = FText::FromString(TEXT("벨 울리기"));
}

bool UCPLabBellComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor) || !IsValid(Interactor)) return false;
	
	// Request 선택 UI 호출
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UCPUIManagerSubsystem* UIManager = GameInstance ? GameInstance->GetSubsystem<UCPUIManagerSubsystem>() : nullptr;
	if (!UIManager || !QuestSelectWidgetClass) return false;
	
	UIManager->PushWidget(QuestSelectWidgetClass);
	return true;
}

bool UCPLabBellComponent::CanInteract() const
{
	const UWorld* World = GetWorld();
	const ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;

	return LabGameMode && !LabGameMode->IsActiveRequest();
}