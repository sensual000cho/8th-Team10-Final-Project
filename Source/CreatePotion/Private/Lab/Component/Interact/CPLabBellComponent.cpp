// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPLabBellComponent.h"

#include "Character/CPInteractionComponent.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/CPNPCSpawner.h"
#include "UI/Widgets/Common/Quest/CPQuestSelectWidget.h"

UCPLabBellComponent::UCPLabBellComponent()
{
	InteractionPrompt = FText::FromString(TEXT("벨 울리기"));
}

void UCPLabBellComponent::BeginFocus(AActor* Interactor)
{
	const UWorld* World = GetWorld();
	const ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;
	
	InteractionPrompt = LabGameMode && LabGameMode->IsActiveRequest() 
		? FText::FromString(TEXT("손님 돌려보내기")) : FText::FromString(TEXT("손님 부르기"));
	
	Super::BeginFocus(Interactor);
}

bool UCPLabBellComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor) || !IsValid(Interactor)) return false;
	
	UWorld* World = GetWorld();
	ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;
	if (!LabGameMode) return false;
	
	// Spawn된 NPC 제거
	if (LabGameMode->IsActiveRequest()){
		ACPNPCSpawner* Spawner = Cast<ACPNPCSpawner>(UGameplayStatics::GetActorOfClass(World, ACPNPCSpawner::StaticClass()));
		if (!Spawner) return false;
		
		Spawner->DestroyActiveNPC();
		if (!LabGameMode->ClearPotionRequest()) return false;
		
		InteractionPrompt = FText::FromString(TEXT("손님 부르기"));
		RefreshUI();
		return true;
	}
	
	// Request 선택 UI 호출
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UCPUIManagerSubsystem* UIManager = GameInstance ? GameInstance->GetSubsystem<UCPUIManagerSubsystem>() : nullptr;
	if (!UIManager || !QuestSelectWidgetClass) return false;
	
	UIManager->PushWidget(QuestSelectWidgetClass);
	return true;
}
