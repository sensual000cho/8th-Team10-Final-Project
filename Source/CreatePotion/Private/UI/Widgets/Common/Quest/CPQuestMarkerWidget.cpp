// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Quest/CPQuestMarkerWidget.h"
#include "Components/TextBlock.h"

void UCPQuestMarkerWidget::SetMarkerState(EQuestMarkerState NewState)
{
	if (!MarkerText) return;

	switch (NewState)
	{
	case EQuestMarkerState::Available:
		MarkerText->SetText(FText::FromString(TEXT("!")));
		SetVisibility(ESlateVisibility::Visible);
		break;
	case EQuestMarkerState::InProgress:
		MarkerText->SetText(FText::FromString(TEXT("...")));   // 진행 중, 원하는 텍스트로 바꾸셔도 됨
		SetVisibility(ESlateVisibility::Visible);
		break;
	case EQuestMarkerState::Completable:
		MarkerText->SetText(FText::FromString(TEXT("?")));
		SetVisibility(ESlateVisibility::Visible);
		break;
	case EQuestMarkerState::None:
	default:
		SetVisibility(ESlateVisibility::Collapsed);
		break;
	}
}

