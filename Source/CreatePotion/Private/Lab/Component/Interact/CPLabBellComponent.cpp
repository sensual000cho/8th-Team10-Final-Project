// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPLabBellComponent.h"
#include "Sound/SoundBase.h"

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

void UCPLabBellComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* World = GetWorld();
	ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;
	if (!LabGameMode) return;

	LabGameMode->OnPotionRequestStateChanged.AddUObject(this, &UCPLabBellComponent::HandlePotionRequestStateChanged);
}

void UCPLabBellComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	ACPLabGameMode* LabGameMode = World ? World->GetAuthGameMode<ACPLabGameMode>() : nullptr;
	if (LabGameMode){
		LabGameMode->OnPotionRequestStateChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
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
		if (BellSound) UGameplayStatics::PlaySoundAtLocation(this, BellSound, GetOwner()->GetActorLocation());
		
		return true;
	}
	
	// Request 선택 UI 호출
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UCPUIManagerSubsystem* UIManager = GameInstance ? GameInstance->GetSubsystem<UCPUIManagerSubsystem>() : nullptr;
	if (!UIManager || !QuestSelectWidgetClass) return false;
	
	UUserWidget* OpenedWidget = UIManager->PushWidget(QuestSelectWidgetClass);
	if (OpenedWidget && BellSound) UGameplayStatics::PlaySoundAtLocation(this, BellSound, GetOwner()->GetActorLocation());
	return true;
}

void UCPLabBellComponent::HandlePotionRequestStateChanged(bool bActiveRequest)
{
	InteractionPrompt = bActiveRequest ? FText::FromString(TEXT("손님 돌려보내기")) : FText::FromString(TEXT("손님 부르기"));
	RefreshUI();
}
