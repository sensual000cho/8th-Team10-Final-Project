#include "NPC/CPQuestNPC.h" 

#include "AIController.h"
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CPInventoryComponent.h"
#include "NPC/Components/CPNPCComponent.h"
#include "NPC/Components/CPNPCMoveComponent.h"
#include "UI/Widgets/Common/Quest/CPQuestMarkerWidget.h"


ACPQuestNPC::ACPQuestNPC() : CurrentQuestId(NAME_None)
{
	InteractableComponent = CreateDefaultSubobject<UCPNPCComponent>(TEXT("InteractableComponent"));
	InteractableComponent->SetupAttachment(RootComponent);
	
	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidget->SetupAttachment(RootComponent);
	InteractionWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	if (const UCPUISettings* UISettings = GetDefault<UCPUISettings>())
	{
		InteractionWidget->SetWidgetClass(UISettings->InteractionWidgetClass.Get());
	}

	QuestMarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("QuestMarkerWidget"));
	QuestMarkerWidget->SetupAttachment(RootComponent);
	QuestMarkerWidget->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	QuestMarkerWidget->SetWidgetSpace(EWidgetSpace::World);
	QuestMarkerWidget->SetTwoSided(true);
}

void ACPQuestNPC::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (InteractionWidget)
	{
		InitialInteractionWidgetLocation = InteractionWidget->GetRelativeLocation();
	}

	if (QuestMarkerWidget)
	{
		InitialQuestMarkerLocation = QuestMarkerWidget->GetRelativeLocation();
	}
}

void ACPQuestNPC::StartDialogue(AActor* Interactor)
{
	if (!IsValid(Interactor) || (ActiveDialogueWidget && ActiveDialogueWidget->IsInViewport())) return;
	
	UGameInstance* GameInstance = GetGameInstance();
	UCPUIManagerSubsystem* UIManager = GameInstance ? GameInstance->GetSubsystem<UCPUIManagerSubsystem>() : nullptr;
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* DialogueWidgetClass = UISettings ? UISettings->NPCDialogueWidgetClass.LoadSynchronous() : nullptr;
	if (!UIManager || !DialogueWidgetClass) return;
	
	// 표시할 Text 선택
	TArray<FText> DialogueLines = FindDialogueLines(Interactor);
	if (CurrentQuestId.IsNone()) return;

	// Widget 띄우기
	ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
	if (!ActiveDialogueWidget){
		CurrentQuestId = NAME_None;
		return;
	}
	
	// Patrol 중지
	if (MoveComponent){
		MoveComponent->StopMoving();
	}
	
	FVector Direction = Interactor->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f;
	if (!Direction.IsNearlyZero()){
		SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
	}
	
	// Player에게 다가감
	if (AAIController* AIController = Cast<AAIController>(GetController())){
		AIController->MoveToActor(
			Interactor,
			StopRadius,
			true,
			true,
			false,
			nullptr,
			true
		);
	}

	ActiveDialogueWidget->OnClosed.RemoveAll(this);
	ActiveDialogueWidget->OnClosed.AddUObject(this, &ACPQuestNPC::OnDialogueClosed);

	RegisterCodex();

	ActiveDialogueWidget->InitDialogueLines(CurrentQuestId, FText::FromName(QuestData->NPCName), DialogueLines);
}

void ACPQuestNPC::SetFocused(bool bFocused)
{
	bIsFocused = bFocused;
	if (!MoveComponent || IsValid(ActiveDialogueWidget)) return;
	
	if (bFocused){
		MoveComponent->StopMoving();
		return;
	}
	
	MoveComponent->StartMoving();
}

void ACPQuestNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!QuestMarkerWidget) return;

	if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		FVector CameraLocation = CameraManager->GetCameraLocation();
		FVector WidgetLocation = QuestMarkerWidget->GetComponentLocation();
		FRotator LookAtRotation = (CameraLocation - WidgetLocation).Rotation();
		QuestMarkerWidget->SetWorldRotation(LookAtRotation);
	}
}

void ACPQuestNPC::HandleThrownPotionImpact(const TArray<FGameplayTag>& PotionEffectTags)
{
	if (PotionEffectTags.IsEmpty()) return;

	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return;

	const FName QuestID = FindAcceptedQuestId();
	if (QuestID.IsNone()) return;

	const FCPQuestResult Result = QuestManager->CreateResult(QuestID, PotionEffectTags);
	CurrentQuestId = Result.QuestId;

	ShowResultDialogue();
}

void ACPQuestNPC::ShowResultDialogue()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return;

	UQuestManager* QuestManager = GameInstance->GetSubsystem<UQuestManager>();
	if (!QuestManager) return;

	if (CurrentQuestId.IsNone()) return;

	FCPQuestResult DeliveryResult = QuestManager->GetResult(CurrentQuestId);
	if (DeliveryResult.QuestId.IsNone()) return;

	const FName QuestID = DeliveryResult.QuestId;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* DialogueWidgetClass = UISettings ? UISettings->NPCDialogueWidgetClass.LoadSynchronous() : nullptr;
	if (!UIManager || !DialogueWidgetClass) return;

	ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
	if (!ActiveDialogueWidget) return;
	
	if (MoveComponent){
		MoveComponent->StopMoving();
	}
	
	ActiveDialogueWidget->OnClosed.RemoveAll(this);
	ActiveDialogueWidget->OnClosed.AddUObject(this, &ACPQuestNPC::OnDialogueClosed);

	TArray<FText> ReactionTexts;
	const TArray<FConditionEvaluation> Evaluations = QuestManager->EvaluateConditions(DeliveryResult.QuestId, DeliveryResult.CurrentEffects);

	for (const FConditionEvaluation& Eval : Evaluations) {
		if (DeliveryResult.DeliveryGrade == EDeliveryGrade::Fail
			&& (!Eval.bWasRequested || Eval.Result != EConditionMatchResult::WrongTag)) continue;

		FText Reaction = QuestManager->GetReactionText(DeliveryResult.QuestId, Eval);
		if (!Reaction.IsEmpty()) {
			ReactionTexts.Add(Reaction);
		}
	}

	ActiveDialogueWidget->InitDialogueLines(QuestID, FText::FromName(QuestData->NPCName), ReactionTexts);
}

void ACPQuestNPC::UpdateOverheadWidgetsScale(float ScaleMultiplier)
{
	Super::UpdateOverheadWidgetsScale(ScaleMultiplier);

	if (QuestMarkerWidget)
	{
		QuestMarkerWidget->SetRelativeLocation(InitialQuestMarkerLocation * ScaleMultiplier);
	}

	if (InteractionWidget)
	{
		InteractionWidget->SetRelativeLocation(InitialInteractionWidgetLocation * ScaleMultiplier);
	}
}

void ACPQuestNPC::BeginPlay()
{
	QuestData = Cast<UCPQuestNPCDataAsset>(NPCData);

	Super::BeginPlay();
	
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UQuestManager* QuestManager = GI->GetSubsystem<UQuestManager>())
		{
			QuestManager->OnQuestUpdated.AddDynamic(this, &ACPQuestNPC::HandleQuestUpdatedForMarker);
		}
	}
	UpdateQuestMarker();
}

FName ACPQuestNPC::FindAcceptedQuestId() const
{
	if (!QuestData) return NAME_None;

	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return NAME_None;

	for (const FName& QuestID : QuestData->QuestIDs) {
		if (QuestID.IsNone()) continue;
		if (QuestManager->GetQuestState(QuestID) != EQuestState::Accepted) continue;
		if (QuestManager->GetQuestCompletionType(QuestID) != EQuestCompletionType::Potion) continue;
		return QuestID;
	}
	return NAME_None;
}

void ACPQuestNPC::OnDialogueClosed()
{
	ActiveDialogueWidget = nullptr;
	CurrentQuestId = NAME_None;
	
	if (!bIsFocused && MoveComponent){
		MoveComponent->StartMoving();
	}
}

void ACPQuestNPC::HandleQuestUpdatedForMarker(FName QuestID, EQuestState NewState)
{
	if (!QuestData || !QuestData->QuestIDs.Contains(QuestID)) return;
	UpdateQuestMarker();
	if (InteractableComponent){
		InteractableComponent->RefreshUI();
	}
}

TArray<FText> ACPQuestNPC::FindDialogueLines(AActor* Interactor)
{
	CurrentQuestId = NAME_None;
	if (!IsValid(Interactor) || !QuestData || QuestData->QuestIDs.IsEmpty()) return {};
	
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return {};
	
	for (const FName& QuestID : QuestData->QuestIDs){
		if (QuestID.IsNone()) continue;
		
		const EQuestState State = QuestManager->GetQuestState(QuestID);
		// 추후 해당 위치에 배치할 텍스트 추가
		if (State == EQuestState::Completed) continue;
		// 퀘스트 수락
		if (State == EQuestState::NotAccepted){
			if (!QuestManager->IsQuestStoryUnlocked(QuestID)) continue;
			
			CurrentQuestId = QuestID;
			return QuestManager->GetQuestScriptLines(QuestID);
		}
		
		const EQuestCompletionType CompletionType = QuestManager->GetQuestCompletionType(QuestID);
		// 대화형 퀘스트
		if (CompletionType == EQuestCompletionType::Dialogue){
			TArray<FText> DialogueLines = QuestManager->GetNPCStoryLines(QuestID);
			if (DialogueLines.IsEmpty() || !QuestManager->CompleteQuestByDialogue(QuestID)) return {};
			
			CurrentQuestId = QuestID;
			return DialogueLines;
		}
		
		// 아이템 수집 퀘스트
		if (CompletionType == EQuestCompletionType::ItemCollection){
			UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
			if (!Inventory) return {};
			
			const TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();
			if (QuestManager->TryCompleteQuestByItem(QuestID, HeldItems)){
				TArray<FName> RequiredItemIDs;
				int32 RequiredItemCount = 0;
				
				QuestManager->GetRequiredItemInfo(QuestID, RequiredItemIDs, RequiredItemCount);
				for (const FName& RequiredItemID : RequiredItemIDs){
					Inventory->RemoveItemByID(RequiredItemID, RequiredItemCount);
				}
				CurrentQuestId = QuestID;
				return QuestManager->GetNPCStoryLines(QuestID);
			}
			
			CurrentQuestId = QuestID;
			return {QuestManager->GetSessionHintText(QuestID)};
		}
		
		CurrentQuestId = QuestID;
		return {QuestManager->GetSessionHintText(QuestID)};
	}
	return {};
}

void ACPQuestNPC::UpdateQuestMarker()
{
	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestData || !QuestManager || !QuestMarkerWidget) return;

	TMap<FName, int32> HeldItemCounts;
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UCPInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UCPInventoryComponent>())
		{
			HeldItemCounts = Inventory->GetItemCountsByID();
		}
	}

	EQuestMarkerState State = QuestManager->GetMarkerStateForQuests(QuestData->QuestIDs, HeldItemCounts);

	if (UCPQuestMarkerWidget* MarkerWidget = Cast<UCPQuestMarkerWidget>(QuestMarkerWidget->GetUserWidgetObject()))
	{
		MarkerWidget->SetMarkerState(State);
	}
}
