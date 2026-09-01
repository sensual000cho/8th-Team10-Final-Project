// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPLabBellComponent.h"

#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Quest/CPQuestSelectWidget.h"

UCPLabBellComponent::UCPLabBellComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionPrompt = FText::FromString(TEXT("벨 울리기"));
	bShowWhenUnavailable = true;
}

bool UCPLabBellComponent::ExecuteInteraction(AActor* Interactor)
{
	if (!CanExecuteInteraction(Interactor)) return false;
	
	// Request 선택 UI 호출
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UCPUIManagerSubsystem* UIManager = GameInstance ? GameInstance->GetSubsystem<UCPUIManagerSubsystem>() : nullptr;
	if (!UIManager || !QuestSelectWidgetClass) return false;
	
	UIManager->PushWidget(QuestSelectWidgetClass);
	return true;
}

bool UCPLabBellComponent::CanExecuteInteraction(AActor* Interactor) const
{
	if (!Super::CanExecuteInteraction(Interactor)) return false;
	
	const UWorld* World = GetWorld();
	const ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;

	return LabGameMode && LabGameMode->GetActiveRequestId().IsNone();
}
