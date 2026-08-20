// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Quest/CPQuestSelectWidget.h"

#include "Components/Button.h"
#include "GameMode/CPLabGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/CPNPCSpawner.h"
#include "Quest/QuestManager.h"

void UCPQuestSelectWidget::BindEvents()
{
	Super::BindEvents();
	
	Button_CallCustomer->OnClicked.AddDynamic(this, &UCPQuestSelectWidget::HandleCallButtonClicked);
	Button_Cancel->OnClicked.AddDynamic(this, &UCPQuestSelectWidget::HandleCancelButtonClicked);
}

void UCPQuestSelectWidget::UnbindEvents()
{
	Super::UnbindEvents();
	
	Button_CallCustomer->OnClicked.RemoveDynamic(this, &UCPQuestSelectWidget::HandleCallButtonClicked);
	Button_Cancel->OnClicked.RemoveDynamic(this, &UCPQuestSelectWidget::HandleCancelButtonClicked);
}

void UCPQuestSelectWidget::HandleCallButtonClicked()
{
	// 임시 QuestId 선택 이후 Player가 조작 가능한 방법으로 변경
	if (!FirstAcceptedQuestId()) return;
	
	if (SelectedQuestId.IsNone()) return;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	ACPNPCSpawner* Spawner = Cast<ACPNPCSpawner>(UGameplayStatics::GetActorOfClass(World, ACPNPCSpawner::StaticClass()));
	if (!Spawner || !Spawner->SpawnNPC((SelectedQuestId))) return;
	
	ACPLabGameMode* LabGameMode = World->GetAuthGameMode<ACPLabGameMode>();
	if (!LabGameMode || !LabGameMode->StartPotionRequest(SelectedQuestId)) return;
	
	RequestClose();
}

void UCPQuestSelectWidget::HandleCancelButtonClicked()
{
	// TODO: UI 닫기 
	//CachedUIManager->CloseWidget(this);
	RequestClose();
}

bool UCPQuestSelectWidget::FirstAcceptedQuestId()
{
	SelectedQuestId = NAME_None;
	
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return false;
	
	const TArray<FName> QuestIds = QuestManager->GetAllTrackedQuestIDs();
	for (const FName& QuestId : QuestIds){
		if (!QuestId.IsNone() && QuestManager->GetQuestState(QuestId) == EQuestState::Accepted){
			SelectedQuestId = QuestId;
			return true;
		}
	}
	
	return false;
}
