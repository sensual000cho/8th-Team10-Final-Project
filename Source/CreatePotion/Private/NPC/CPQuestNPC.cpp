#include "NPC/CPQuestNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"       
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "NPC/CPNPCMoveComponent.h"
#include "NPC/CPNPCAIController.h"
#include "Character/CPInteractionComponent.h"                   
#include "Components/WidgetComponent.h"                        
#include "Kismet/GameplayStatics.h"                             
#include "Components/CPInventoryComponent.h"                    
#include "UI/Widgets/Common/Quest/CPQuestMarkerWidget.h"                 


ACPQuestNPC::ACPQuestNPC() : CurrentQuestId(NAME_None)
{
	InteractionRefreshInterval = 0.1f;

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

bool ACPQuestNPC::CanInteract(AActor* Interactor) const
{
	return true;
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

FText ACPQuestNPC::GetInteractionPrompt() const
{
	return FText::FromString(TEXT("대화하기"));
}

void ACPQuestNPC::RefreshInteractionState()
{
	Super::RefreshInteractionState();
	UpdateGazeTracking();
}

void ACPQuestNPC::UpdateGazeTracking()
{
	if (bIsInteracting)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!MoveComponent)
	{
		MoveComponent = FindComponentByClass<UCPNPCMoveComponent>();
	}

	bool bLookedAtNow = false;
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			if (UCPInteractionComponent* PlayerInteraction = PlayerPawn->FindComponentByClass<UCPInteractionComponent>())
			{
				bLookedAtNow = (PlayerInteraction->GetCurrentTarget() == this);
			}
		}
	}

	if (bLookedAtNow)
	{
		World->GetTimerManager().ClearTimer(TraceLostTimerHandle);

		if (!bIsPlayerLooking)
		{
			bIsPlayerLooking = true;

			if (MoveComponent)
			{
				MoveComponent->StopMoving();
			}
		}
		return;
	}

	if (bIsPlayerLooking && !World->GetTimerManager().IsTimerActive(TraceLostTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			TraceLostTimerHandle,
			this,
			&ACPQuestNPC::OnTraceFocusLost,
			0.5f,
			false
		);
	}
}

void ACPQuestNPC::HandleInteract(AActor* Interactor)
{
	if (!Interactor) { return; }

	bIsInteracting = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TraceLostTimerHandle);
	}

	if (MoveComponent)
	{
		MoveComponent->StopMoving();
	}

	FVector Direction = Interactor->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f;
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(FRotationMatrix::MakeFromX(Direction).Rotator());
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
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

	bIsInteracting = true;
	if (MoveComponent)
	{
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

const TArray<FName>& ACPQuestNPC::GetPotionQuestIDs() const
{
	static const TArray<FName> QuestIDs;
	return QuestIDs;
}

FName ACPQuestNPC::FindAcceptedQuestId() const
{
	if (!QuestData) return NAME_None;

	UGameInstance* GameInstance = GetGameInstance();
	UQuestManager* QuestManager = GameInstance ? GameInstance->GetSubsystem<UQuestManager>() : nullptr;
	if (!QuestManager) return NAME_None;

	for (const FName& QuestID : GetPotionQuestIDs()) {
		if (QuestID.IsNone()) continue;
		if (QuestManager->GetQuestState(QuestID) != EQuestState::Accepted) continue;

		return QuestID;
	}
	return NAME_None;
}

void ACPQuestNPC::OnTraceFocusLost()
{
	if (bIsInteracting) { return; }

	bIsPlayerLooking = false;

	if (MoveComponent)
	{
		MoveComponent->StartMoving();
	}
}

void ACPQuestNPC::OnDialogueClosed()
{
	bIsInteracting = false;
	ActiveDialogueWidget = nullptr;
}

void ACPQuestNPC::HandleQuestUpdatedForMarker(FName QuestID, EQuestState NewState)
{
	UpdateQuestMarker();
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

	EQuestMarkerState State = QuestManager->GetMarkerStateForQuests(GetPotionQuestIDs(), HeldItemCounts);

	if (UCPQuestMarkerWidget* MarkerWidget = Cast<UCPQuestMarkerWidget>(QuestMarkerWidget->GetUserWidgetObject()))
	{
		MarkerWidget->SetMarkerState(State);
	}
}