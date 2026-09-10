#include "NPC/CPQuestNPC.h"
#include "Data/NPC/CPNPCDataAsset.h"       
#include "Data/NPC/CPQuestNPCDataAsset.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Quest/QuestManager.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "NPC/CPNPCMoveComponent.h"
#include "NPC/CPNPCAIController.h"

ACPQuestNPC::ACPQuestNPC() : CurrentQuestId(NAME_None)
{
}

bool ACPQuestNPC::CanInteract_Implementation(AActor* Interactor)
{
	if (!Interactor)
	{
		return true;
	}

	if (!bIsInteracting)
	{
		if (!MoveComponent)
		{
			MoveComponent = FindComponentByClass<UCPNPCMoveComponent>();
		}

		if (!bIsPlayerLooking)
		{
			bIsPlayerLooking = true;

			//UE_LOG(LogTemp, Error, TEXT("[%s] 시선 감지 -> StopMoving 호출"), *GetNameSafe(this));
			if (MoveComponent)
			{
				MoveComponent->StopMoving();
			}
		}

		if (UWorld* World = GetWorld())
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

	return true;
}

FText ACPQuestNPC::GetInteractionPrompt_Implementation()
{
	return FText::FromString(TEXT("대화하기"));
}

void ACPQuestNPC::OnInteract_Implementation(AActor* Interactor)
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
			true,  // bStopOnOverlap
			true,  // bUsePathfinding
			false, // bCanStrafe
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
	
	TArray<FText> ReactionTexts;
	const TArray<FConditionEvaluation> Evaluations = QuestManager->EvaluateConditions(DeliveryResult.QuestId, DeliveryResult.CurrentEffects);
	
	for (const FConditionEvaluation& Eval : Evaluations){
		if (DeliveryResult.DeliveryGrade == EDeliveryGrade::Fail 
			&&(!Eval.bWasRequested || Eval.Result != EConditionMatchResult::WrongTag)) continue;
		
		FText Reaction = QuestManager->GetReactionText(DeliveryResult.QuestId, Eval);
		if (!Reaction.IsEmpty()){
			ReactionTexts.Add(Reaction);
		}
	}
	
	ActiveDialogueWidget->InitDialogueLines(QuestID, FText::FromName(QuestData->NPCName), ReactionTexts);
}

void ACPQuestNPC::BeginPlay()
{
	Super::BeginPlay();
	
	QuestData = Cast<UCPQuestNPCDataAsset>(NPCData);
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
	
	for (const FName& QuestID : GetPotionQuestIDs()){
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