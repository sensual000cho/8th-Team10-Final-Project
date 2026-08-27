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
	Button_CallCustomer->OnClicked.RemoveDynamic(this, &UCPQuestSelectWidget::HandleCallButtonClicked);
	Button_Cancel->OnClicked.RemoveDynamic(this, &UCPQuestSelectWidget::HandleCancelButtonClicked);
	Super::UnbindEvents();
}

void UCPQuestSelectWidget::HandleCallButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[QuestSelectWidget] HandleCallButtonClicked 함수 진입"));
	// 임시 QuestId 선택 이후 Player가 조작 가능한 방법으로 변경
	if (SelectedQuestId.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestSelectWidget] SelectedQuestId가 비어있습니다"));
		return;
	}
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	ACPNPCSpawner* Spawner = Cast<ACPNPCSpawner>(UGameplayStatics::GetActorOfClass(World, ACPNPCSpawner::StaticClass()));
	if (!Spawner || !Spawner->SpawnNPC((SelectedQuestId)))
	{
		UE_LOG(LogTemp, Error, TEXT("[QuestSelectWidget] NPC 스포너가 설치되어있지 않습니다."));
		return;
	}
	
	ACPLabGameMode* LabGameMode = World->GetAuthGameMode<ACPLabGameMode>();
	if (!LabGameMode || !LabGameMode->StartPotionRequest(SelectedQuestId)) return;
	
	RequestClose();
}

void UCPQuestSelectWidget::HandleCancelButtonClicked()
{
	RequestClose();
}
