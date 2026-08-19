// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPLabBellComponent.h"

#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"
#include "UI/Widgets/Common/Quest/CPQuestSelectWidget.h"

UCPLabBellComponent::UCPLabBellComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionPrompt = FText::FromString(TEXT("벨 울리기"));
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
	const ACPLabGameState* LabGameState = World ? World->GetGameState<ACPLabGameState>() : nullptr;
	const UCPLabPotionSessionComponent* Session = LabGameState ? LabGameState->GetPotionSession() : nullptr;
	
	return Session && !Session->HasActiveRequest();
}
