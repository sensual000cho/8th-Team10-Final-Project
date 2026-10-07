#include "UI/Widgets/Common/Quest/CPQuestMarkerWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UCPQuestMarkerWidget::SetMarkerState(EQuestMarkerState NewState)
{
	if (!MarkerImage) return;

	UTexture2D* Texture = nullptr;
	switch (NewState)
	{
	case EQuestMarkerState::Available:
		Texture = AvailableTexture;
		break;
	case EQuestMarkerState::InProgress:
		Texture = InProgressTexture;
		break;
	case EQuestMarkerState::Completable:
		Texture = CompletableTexture;
		break;
	default:
		break;
	}

	// 텍스처가 없으면(None 상태 또는 미지정) 숨김
	if (!Texture)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	MarkerImage->SetBrushFromTexture(Texture, true);
	SetVisibility(ESlateVisibility::Visible);
}