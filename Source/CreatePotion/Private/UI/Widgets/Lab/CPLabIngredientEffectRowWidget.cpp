// 재료 효과 한 줄의 값 갱신과 변화 상태 판정을 구현한다.

#include "UI/Widgets/Lab/CPLabIngredientEffectRowWidget.h"

#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UCPLabIngredientEffectRowWidget::SetEffectData(const FText& InEffectName)
{
	if (Text_EffectName)
	{
		Text_EffectName->SetText(InEffectName);
	}

	if (Text_CurrentLevel)
	{
		Text_CurrentLevel->SetText(FText::GetEmpty());
	}

	DeltaState = ECPLabIngredientEffectDeltaState::None;

	if (Text_PreviewLevel)
	{
		Text_PreviewLevel->SetText(FText::GetEmpty());
	}

	if (PreviewGroup)
	{
		PreviewGroup->SetVisibility(ESlateVisibility::Collapsed);
	}

	BP_ApplyDeltaStyle(DeltaState);
}

ECPLabIngredientEffectDeltaState UCPLabIngredientEffectRowWidget::GetDeltaState() const
{
	return DeltaState;
}

FText UCPLabIngredientEffectRowWidget::FormatLevel(int32 Level)
{
	return FText::FromString(FString::Printf(TEXT("Lv. %d"), Level));
}
