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
#include "UI/Widgets/Lab/CPLabResultWidget.h"
#include "Quest/Components/CPQuestLocationComponent.h"


ACPQuestNPC::ACPQuestNPC() : CurrentQuestId(NAME_None)
{
	InteractableComponent = CreateDefaultSubobject<UCPNPCComponent>(TEXT("InteractableComponent"));
	InteractableComponent->SetupAttachment(RootComponent);

	QuestMarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("QuestMarkerWidget"));
	QuestMarkerWidget->SetupAttachment(RootComponent);
	QuestMarkerWidget->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	QuestMarkerWidget->SetWidgetSpace(EWidgetSpace::World);
	QuestMarkerWidget->SetTwoSided(true);
}

void ACPQuestNPC::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (QuestMarkerWidget) {
		InitialQuestMarkerLocation = QuestMarkerWidget->GetRelativeLocation();
	}
}

void ACPQuestNPC::StartDialogue(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] StartDialogue 진입"), *GetName());

	if (!IsValid(Interactor) || (ActiveDialogueWidget && ActiveDialogueWidget->IsInViewport())) return;

	UGameInstance* GameInstance = GetGameInstance();
	UCPUIManagerSubsystem* UIManager = GameInstance ? GameInstance->GetSubsystem<UCPUIManagerSubsystem>() : nullptr;
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* DialogueWidgetClass = UISettings ? UISettings->NPCDialogueWidgetClass.LoadSynchronous() : nullptr;
	if (!UIManager || !DialogueWidgetClass) return;

	// 대화 중에는 퀘스트 위치 이동 보류 (대화로 완료되는 순간 사라지는 것 방지)
	UCPQuestLocationComponent* LocationComp = FindComponentByClass<UCPQuestLocationComponent>();
	if (LocationComp)
	{
		LocationComp->SetRefreshLocked(true);
	}

	// 표시할 Text 선택

	TArray<FCPDialogueLine> DialogueLines = FindDialogueLines(Interactor);
	if (DialogueLines.IsEmpty())
	{
		if (LocationComp)
		{
			LocationComp->SetRefreshLocked(false);
		}
		return;
	}

	// Widget 띄우기
	ActiveDialogueWidget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
	if (!ActiveDialogueWidget) {
		CurrentQuestId = NAME_None;
		PendingResultQuestId = NAME_None;
		if (LocationComp)
		{
			LocationComp->SetRefreshLocked(false);
		}
		return;
	}

	// Patrol 중지
	if (MoveComponent) {
		MoveComponent->StopMoving();
	}

	FVector Direction = Interactor->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f;
	if (!Direction.IsNearlyZero()) {
		SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
	}

	// Player에게 다가감 (퀘스트 위치에 있을 때는 제자리 유지)
	const bool bAwayFromHome = LocationComp && LocationComp->IsAwayFromHome();
	if (!bAwayFromHome)
	{
		if (AAIController* AIController = Cast<AAIController>(GetController())) {
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

	if (bFocused) {
		MoveComponent->StopMoving();
		return;
	}

	// 퀘스트 위치에 있으면 순찰 재개하지 않음
	const UCPQuestLocationComponent* LocationComp = FindComponentByClass<UCPQuestLocationComponent>();
	if (LocationComp && LocationComp->IsAwayFromHome()) return;

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

	if (MoveComponent) {
		MoveComponent->StopMoving();
	}

	ActiveDialogueWidget->OnClosed.RemoveAll(this);
	ActiveDialogueWidget->OnClosed.AddUObject(this, &ACPQuestNPC::OnDialogueClosed);

	TArray<FCPDialogueLine> ReactionTexts;
	const TArray<FConditionEvaluation> Evaluations = QuestManager->EvaluateConditions(DeliveryResult.QuestId, DeliveryResult.CurrentEffects);

	for (const FConditionEvaluation& Eval : Evaluations) {
		if (DeliveryResult.DeliveryGrade == EDeliveryGrade::Fail
			&& (!Eval.bWasRequested || Eval.Result != EConditionMatchResult::WrongTag)) continue;

		FText Reaction = QuestManager->GetReactionText(DeliveryResult.QuestId, Eval);
		if (!Reaction.IsEmpty()) {
			FCPDialogueLine Line;
			Line.Text = Reaction;
			ReactionTexts.Add(Line);
		}
	}

	ActiveDialogueWidget->InitDialogueLines(QuestID, FText::FromName(QuestData->NPCName), ReactionTexts);
}

void ACPQuestNPC::UpdateOverheadWidgetsScale(float ScaleMultiplier)
{
	Super::UpdateOverheadWidgetsScale(ScaleMultiplier);
	if (QuestMarkerWidget) {
		QuestMarkerWidget->SetRelativeLocation(InitialQuestMarkerLocation * ScaleMultiplier);
	}
}

void ACPQuestNPC::SetQuestMarkerVisibility(bool bVisible)
{
	if (QuestMarkerWidget)
	{
		QuestMarkerWidget->SetVisibility(bVisible);
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

	// 퀘스트 위치 변경 시 순찰 제어
	if (UCPQuestLocationComponent* LocationComp = FindComponentByClass<UCPQuestLocationComponent>())
	{
		LocationComp->OnLocationChanged.AddWeakLambda(this, [this](bool bAwayFromHome)
			{
				if (!MoveComponent) return;

				if (bAwayFromHome)
				{
					MoveComponent->StopMoving();
				}
				else if (!bIsFocused && !IsValid(ActiveDialogueWidget) && MoveComponent->IsMovePaused())
				{
					MoveComponent->StartMoving();
				}
			});

		// 이동 페이드가 끝나면 대기 중이던 결과 UI 표시
		LocationComp->OnFadeFinished.AddUObject(this, &ACPQuestNPC::HandleLocationFadeFinished);

		// 레벨 재진입으로 이미 퀘스트 위치에 있으면 순찰 중지
		if (LocationComp->IsAwayFromHome() && MoveComponent)
		{
			MoveComponent->StopMoving();
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
	UE_LOG(LogTemp, Log, TEXT("[%s] OnDialogueClosed 호출"), *GetName());

	ActiveDialogueWidget = nullptr;
	CurrentQuestId = NAME_None;

	const FName ResultQuestId = PendingResultQuestId;
	PendingResultQuestId = NAME_None;

	// 대화 중 보류된 위치 이동 적용 (이동이 필요하면 여기서 페이드가 시작됨)
	UCPQuestLocationComponent* LocationComp = FindComponentByClass<UCPQuestLocationComponent>();
	if (LocationComp)
	{
		LocationComp->SetRefreshLocked(false);
	}

	// 퀘스트 위치에 있으면 순찰하지 않음 (복귀 시 이미 재개됐으면 중복 호출 방지)
	const bool bAwayFromHome = LocationComp && LocationComp->IsAwayFromHome();
	if (!bIsFocused && !bAwayFromHome && MoveComponent && MoveComponent->IsMovePaused()) {
		MoveComponent->StartMoving();
	}

	// 대화형 퀘스트 완료 → 결과 UI (페이드 중이면 끝난 뒤에 표시)
	if (!ResultQuestId.IsNone())
	{
		if (LocationComp && LocationComp->IsFading())
		{
			ResultAfterFadeQuestId = ResultQuestId;
		}
		else
		{
			ShowQuestResultWidget(ResultQuestId);
		}
	}
}

void ACPQuestNPC::HandleLocationFadeFinished()
{
	if (ResultAfterFadeQuestId.IsNone()) return;

	const FName QuestId = ResultAfterFadeQuestId;
	ResultAfterFadeQuestId = NAME_None;

	ShowQuestResultWidget(QuestId);
}


void ACPQuestNPC::HandleQuestUpdatedForMarker(FName QuestID, EQuestState NewState)
{
	if (!QuestData || !QuestData->QuestIDs.Contains(QuestID)) return;
	UpdateQuestMarker();
	if (InteractableComponent) {
		InteractableComponent->RefreshUI();
	}
}

void ACPQuestNPC::HandleInventoryUpdatedForMarker()
{
	UpdateQuestMarker();
}

TArray<FCPDialogueLine> ACPQuestNPC::FindDialogueLines(AActor* Interactor)
{
	CurrentQuestId = NAME_None;
	PendingResultQuestId = NAME_None;
	if (!IsValid(Interactor) || !QuestData || QuestData->QuestIDs.IsEmpty()) return {};

	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return {};

	for (const FName& QuestID : QuestData->QuestIDs) {
		if (QuestID.IsNone()) continue;

		const EQuestState State = QuestManager->GetQuestState(QuestID);
		// 추후 해당 위치에 배치할 텍스트 추가
		if (State == EQuestState::Completed) continue;
		// 퀘스트 수락
		if (State == EQuestState::NotAccepted) {
			if (!QuestManager->IsQuestStoryUnlocked(QuestID)) continue;

			CurrentQuestId = QuestID;
			return QuestManager->GetQuestScriptLines(QuestID);
		}

		const EQuestCompletionType CompletionType = QuestManager->GetQuestCompletionType(QuestID);
		// 대화형 퀘스트
		if (CompletionType == EQuestCompletionType::Dialogue) {
			TArray<FCPDialogueLine> DialogueLines = QuestManager->GetNPCStoryLines(QuestID);

			if (DialogueLines.IsEmpty() || !QuestManager->CompleteQuestByDialogue(QuestID)) return {};

			CurrentQuestId = QuestID;
			PendingResultQuestId = QuestID;
			return DialogueLines;
		}

		// 아이템 수집 퀘스트
		if (CompletionType == EQuestCompletionType::ItemCollection) {
			UCPInventoryComponent* Inventory = Interactor->FindComponentByClass<UCPInventoryComponent>();
			if (!Inventory) return {};

			const TMap<FName, int32> HeldItems = Inventory->GetItemCountsByID();
			if (QuestManager->TryCompleteQuestByItem(QuestID, HeldItems)) {
				if (QuestManager->ShouldConsumeRequiredItems(QuestID))
				{
					TArray<FName> RequiredItemIDs;
					int32 RequiredItemCount = 0;

					QuestManager->GetRequiredItemInfo(QuestID, RequiredItemIDs, RequiredItemCount);
					for (const FName& RequiredItemID : RequiredItemIDs) {
						Inventory->RemoveItemByID(RequiredItemID, RequiredItemCount);
					}
				}
				CurrentQuestId = QuestID;
				return QuestManager->GetNPCStoryLines(QuestID);
			}

			FCPDialogueLine HintLine;
			HintLine.Text = QuestManager->GetSessionHintText(QuestID);
			CurrentQuestId = QuestID;
			return { HintLine };
		}

		FCPDialogueLine HintLine;
		HintLine.Text = QuestManager->GetSessionHintText(QuestID);
		CurrentQuestId = QuestID;
		return { HintLine };
	}
	
	bool bHasValidQuest = false;
	
	for (const FName& QuestID : QuestData->QuestIDs){
		if (QuestID.IsNone()) continue;
		
		bHasValidQuest = true;
		if (QuestManager->GetQuestState(QuestID) != EQuestState::Completed) return {};
	}
	
	if (bHasValidQuest){
		return QuestData->CompletedDialogueLines;
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
			Inventory->OnContainerUpdated.AddUniqueDynamic(this, &ACPQuestNPC::HandleInventoryUpdatedForMarker);
		}
	}

	EQuestMarkerState State = QuestManager->GetMarkerStateForQuests(QuestData->QuestIDs, HeldItemCounts);

	if (UCPQuestMarkerWidget* MarkerWidget = Cast<UCPQuestMarkerWidget>(QuestMarkerWidget->GetUserWidgetObject()))
	{
		MarkerWidget->SetMarkerState(State);
	}
}

void ACPQuestNPC::ShowQuestResultWidget(FName QuestID)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	UClass* ResultWidgetClass = UISettings ? UISettings->ResultWidgetClass.LoadSynchronous() : nullptr;

	if (UIManager && ResultWidgetClass)
	{
		UCPLabResultWidget* ResultWidget = Cast<UCPLabResultWidget>(UIManager->PushWidget(ResultWidgetClass));
		if (ResultWidget && !ResultWidget->InitializeResult(QuestID))
		{
			ResultWidget->RequestClose();
		}
	}
}