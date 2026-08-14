// CPLabHUD.cpp

#include "UI/HUD/CPLabHUD.h"

#include "Character/CPPlayerController.h"
#include "Components/CPLabContainerComponent.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "GameMode/CPLabGameMode.h"
#include "Lab/Actor/CPLabContainerActor.h"
#include "UI/Widgets/Lab/CPLabIngredientSelectWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagRangeWidget.h"
#include "UI/Widgets/Lab/TagChoice/CPTagSelectionWidget.h"


void ACPLabHUD::OnMainHUDWidgetCreated()
{
	Super::OnMainHUDWidgetCreated();
	
	//TODO: 퀘스트 저널 토글 델리게이트 바인딩
}

void ACPLabHUD::StartLabCraftingFlow()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager || !TagSelectionWidgetClass) return;
	
	UCPTagSelectionWidget* CreatedWidget = Cast<UCPTagSelectionWidget>(UIManager->FindOpenWidget(TagSelectionWidgetClass));
	if (CreatedWidget)
	{
		if (UCPTagSelectionWidget* SelectionWidget = Cast<UCPTagSelectionWidget>(CreatedWidget))
		{
			SelectionWidget->OnTagSelectionConfirmed.AddDynamic(this, &ACPLabHUD::HandleTagSelectionConfirmed);
			UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 태그 선택 확정 델리게이트 바인딩 성공!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 태그 선택 위젯 바인딩 실패 - 위젯을 찾지 못함"));
	}
	
}

void ACPLabHUD::BindTagSelectionWidget(UCPTagSelectionWidget* TargetWidget)
{
	if (TargetWidget)
	{
		TargetWidget->OnTagSelectionConfirmed.AddUniqueDynamic(this, &ACPLabHUD::HandleTagSelectionConfirmed);
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] TagSelectionWidget 델리게이트 바인딩 완료"))
	}
}

void ACPLabHUD::BindTagRangeWidget(UCPTagRangeWidget* TargetWidget)
{
	if (TargetWidget)
	{
		TargetWidget->OnTagRangeConfirmed.AddUniqueDynamic(this, &ACPLabHUD::HandleTagRangeConfirmed);
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] TagRangeWidget 델리게이트 바인딩 완료"))
	}
}

void ACPLabHUD::BindLabIngredientSelectWidget(UCPLabIngredientSelectWidget* TargetWidget)
{
	if (TargetWidget)
	{
		TargetWidget->OnIngredientConfirmed.AddUniqueDynamic(this, &ACPLabHUD::HandleIngredientSelectionConfirmed);
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] IngredientSelectionWidget 델리게이트 바인딩 완료"))
	}
}

void ACPLabHUD::HandleTagRangeConfirmed()
{
	UE_LOG(LogTemp, Warning, TEXT("[LabHUD] HandleTagConfirmed 호출됨"));
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager) return;
	
	if (LabIngredientSelectWidgetClass)
	{
		UUserWidget* CreatedWidget = UIManager->PushWidget(LabIngredientSelectWidgetClass);
		
		if (UCPLabIngredientSelectWidget* IngredientWidget = Cast<UCPLabIngredientSelectWidget>(CreatedWidget))
		{
			BindLabIngredientSelectWidget(IngredientWidget);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("[HUD] 태그 범위 설정 완료 -> 재료 선택 창 정상 오픈!"));
	}
	else
	{
		// 클래스 할당 안됨
		UE_LOG(LogTemp, Error, TEXT("[HUD] LabIngredientWidgetClass가 비어있습니다! BP_HUD에서 클래스를 지정해주세요."));
		return;
	}
	
	if (ACPPlayerController* PC = Cast<ACPPlayerController>(GetOwningPlayerController()))
	{
		PC->ToggleLabUI();
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 재료 선택 UI 열기 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 재료 선택 UI 열기 성공"));
	}
	
	
}

void ACPLabHUD::HandleTagSelectionConfirmed(const FTagSelectionData& SelectionData)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager || !TagRangeWidgetClass) return;
	
	UUserWidget* CreatedWidget = UIManager->PushWidget(TagRangeWidgetClass);
	
	if (UCPTagRangeWidget* RangeWidget = Cast<UCPTagRangeWidget>(CreatedWidget))
	{
		RangeWidget->InitTagRangeWidget(
			SelectionData.QuestID,
			SelectionData.SelectedTags,
			SelectionData.SavedValues
		);
		
		BindTagRangeWidget(RangeWidget);
		
		UE_LOG(LogTemp, Warning, TEXT("[LabHUD] 태그 범위 위젯 오픈 및 초기화 완료!"));
	}
}

void ACPLabHUD::HandleIngredientSelectionConfirmed()
{
	// 게임모드 가져오기
	ACPLabGameMode* GM = Cast<ACPLabGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GM)
	{
		UE_LOG(LogTemp, Error, TEXT("[LabHUD] LabGameMode 찾을 수 없음"));
		return;
	}
	
	// LabContainer 찾기
	AActor* LabActor = UGameplayStatics::GetActorOfClass(GetWorld(), ACPLabContainerActor::StaticClass());
	UCPLabContainerComponent* LabContainer = LabActor ? LabActor->FindComponentByClass<UCPLabContainerComponent>() : nullptr;
	
	if (!LabContainer)
	{
		UE_LOG(LogTemp, Error, TEXT("[LabHUD] 월드에서 LabContainer를 찾을 수 없음."));
		return;
	}
	
	const TArray<UCPForageableItemData*>& SelectedItems = LabContainer->GetIngredientsData();
	GM->SetIngredientsDataAsset(SelectedItems);
}

void ACPLabHUD::HandleQuestJournalToggle()
{
	UE_LOG(LogTemp, Warning, TEXT("[LabHUD] HandleQuestJournalToggle 호출됨"));
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UCPUIManagerSubsystem* UIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager) return;
	
	if (QuestJournalWidgetClass)
	{
		UIManager->ToggleWidget(QuestJournalWidgetClass);	
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[LabHUD] QuestJournalWidgetClass 미할당"));
	}
}
