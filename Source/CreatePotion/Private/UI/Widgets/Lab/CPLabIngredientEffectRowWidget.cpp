// 재료 효과 한 줄의 값 갱신과 변화 상태 판정을 구현한다.

#include "UI/Widgets/Lab/CPLabIngredientEffectRowWidget.h"

#include "Components/TextBlock.h"
#include "Components/Widget.h"

#define LOCTEXT_NAMESPACE "CPLabIngredientEffectRowWidget"

void UCPLabIngredientEffectRowWidget::SetEffectData(
	const FText& InEffectName,
	int32 InCurrentLevel,
	bool bInHasPreview,
	int32 InPreviewLevel)
{
	if (Text_EffectName)
	{
		Text_EffectName->SetText(InEffectName);
	}

	if (Text_CurrentLevel)
	{
		Text_CurrentLevel->SetText(FormatLevel(InCurrentLevel));
	}

	if (!bInHasPreview)
	{
		// 예상값이 없을 때는 0을 표시하지 않고 예상값 묶음 전체를 레이아웃에서 제거한다.
		DeltaState = ECPLabIngredientEffectDeltaState::None;

		if (PreviewGroup)
		{
			PreviewGroup->SetVisibility(ESlateVisibility::Collapsed);
		}

		BP_ApplyDeltaStyle(DeltaState);
		return;
	}

	if (InPreviewLevel > InCurrentLevel)
	{
		DeltaState = ECPLabIngredientEffectDeltaState::Increased;
	}
	else if (InPreviewLevel < InCurrentLevel)
	{
		DeltaState = ECPLabIngredientEffectDeltaState::Decreased;
	}
	else
	{
		DeltaState = ECPLabIngredientEffectDeltaState::Unchanged;
	}

	if (Text_PreviewLevel)
	{
		Text_PreviewLevel->SetText(FormatLevel(InPreviewLevel));
	}

	if (PreviewGroup)
	{
		PreviewGroup->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	// C++은 변화 상태만 판정하고 실제 색상과 화살표 표현은 WBP에 위임한다.
	BP_ApplyDeltaStyle(DeltaState);
}

ECPLabIngredientEffectDeltaState UCPLabIngredientEffectRowWidget::GetDeltaState() const
{
	return DeltaState;
}

FText UCPLabIngredientEffectRowWidget::FormatLevel(int32 Level)
{
	return FText::Format(
		LOCTEXT("EffectLevelFormat", "Lv. {0}"),
		FText::AsNumber(Level));
}

#undef LOCTEXT_NAMESPACE
