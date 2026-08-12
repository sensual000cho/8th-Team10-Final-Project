#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "GameplayTagContainer.h"
#include "Quest/QuestTypes.h"
#include "CPTagRangeWidget.generated.h"

USTRUCT(BlueprintType)
struct FTagConfirmData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResultStr; // "O", "X", "UP", "DOWN"
};

class UScrollBox;
class UTextBlock;
class UButton;
class UCPTagRangeEntryWidget;
class UCPHintWidget;
class UCPTagSelectionWidget;
class UCPContainerMainWidget;
class UCPLabIngredientSelectWidget;

UCLASS()
class CREATEPOTION_API UCPTagRangeWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()

public:
	// 이전 UI(태그 선택)에서 전달한 정보로 위젯 초기화 (저장된 수치 포함)
	void InitTagRangeWidget(FName InQuestID, const TArray<FGameplayTag>& InSelectedTags, const TMap<FGameplayTag, int32>& InSavedValues = {});

	// 확정된 태그와 지정된 범위, 판정 결과를 모두 모아서 반환
	UFUNCTION(BlueprintCallable, Category = "Alchemy")
	TArray<FTagConfirmData> GetFinalizedTagValues() const;

protected:
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;

	UFUNCTION()
	void OnEvaluateClicked(); // "이건 어떠세요?" 버튼 클릭 시

	UFUNCTION()
	void OnConfirmClicked();  // "확정하기" 버튼 클릭 시

	UFUNCTION()
	void OnBackClicked(); // "뒤로 가기" 버튼 클릭 시

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_Entries;

	UPROPERTY(meta = (BindWidget))
	UCPHintWidget* UI_HintWidget;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Evaluate; // "이건 어떠세요?"

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Confirm;  // "확정하기"

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPTagRangeEntryWidget> EntryWidgetClass;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Button_Back; // 뒤로 가기

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPTagSelectionWidget> TagSelectionWidgetClass;

private:
	FName CurrentQuestID;
	TArray<FGameplayTag> SelectedTags;
	TMap<FGameplayTag, int32> SavedTagValues; // 이전 설정 수치 기억용
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCPLabIngredientSelectWidget> LabIngredientWidgetClass;
};