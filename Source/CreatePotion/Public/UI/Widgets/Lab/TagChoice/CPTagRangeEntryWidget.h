#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/Border.h"
#include "CPTagRangeEntryWidget.generated.h"

class UTextBlock;
class USlider;
class UHorizontalBox;

UCLASS()
class CREATEPOTION_API UCPTagRangeEntryWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	void InitEntry(const FGameplayTag& InTag, int32 InitialValue = 0);
	int32 GetCurrentSliderValue() const;
	void SetFeedbackText(const FString& FeedbackStr);
	FGameplayTag GetEntryTag() const { return CurrentTag; }

protected:
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;

	UFUNCTION()
	void OnSliderValueChanged(float NewValue);

	// 블록 색상을 동적으로 채워주는 함수
	void UpdateBlockColors(int32 Value);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_TagName;

	UPROPERTY(meta = (BindWidget))
	USlider* Slider_Value;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HBox_Blocks;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Feedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Color")
	FLinearColor ActiveColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Color")
	FLinearColor InactiveColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.3f);

private:
	FGameplayTag CurrentTag;
};