// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPForageableCodexWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CPForageableItemData.h"
#include "Internationalization/StringTable.h"

UCPForageableCodexWidget::UCPForageableCodexWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CodexStringTableId(TEXT("ST_ForageableCodex"))
	, CurrentEntryIndex(0)
	, CurrentTextIndex(0)
{
}

void UCPForageableCodexWidget::ShowPreviousEntry()
{
	if (Entries.Num() <= 0) return;
	
	CurrentEntryIndex--;
	if (CurrentEntryIndex < 0) CurrentEntryIndex += Entries.Num();
	CurrentTextIndex = 0;
	
	RefreshCodex();
}

void UCPForageableCodexWidget::ShowNextEntry()
{
	if (Entries.Num() <= 0) return;
	
	CurrentEntryIndex = (CurrentEntryIndex+1) % Entries.Num();
	CurrentTextIndex = 0;
	
	RefreshCodex();
}

void UCPForageableCodexWidget::SetTextIndex(int32 Index)
{
	CurrentTextIndex = Index;
	RefreshCodex();
}

void UCPForageableCodexWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	RefreshCodex();
}

void UCPForageableCodexWidget::RefreshCodex()
{
	// 해당 번호의 Index가 유효하지 않은 경우
	if (!Entries.IsValidIndex(CurrentEntryIndex)){
		if (CodexImage){
			CodexImage->SetBrushFromTexture(nullptr);
		}
		
		if (NameText){
			NameText->SetText(FText::GetEmpty());
		}
		
		if (DescriptionText){
			DescriptionText->SetText(FText::GetEmpty());
		}
		return;
	}
	
	const UCPForageableItemData* CurrentEntry = Entries[CurrentEntryIndex];
	if (!CurrentEntry) return;
	
	if (CodexImage){
		UTexture2D* CodexImageTexture = CurrentEntry->CodexImage.LoadSynchronous();
		CodexImage->SetBrushFromTexture(CodexImageTexture);
	}
	
	if (NameText){
		NameText->SetText(CurrentEntry->DisplayName);
	}
	
	if (!DescriptionText) return;
	
	if (!CurrentEntry->CodexTextKeys.IsEmpty())	{
		// 현 시점 index 0만 사용
		const FName CodexKeyPrefix = CurrentEntry->CodexTextKeys[0];

		const FString TextKeyString = FString::Printf(
			TEXT("%s.%02d"),
			*CodexKeyPrefix.ToString(),
			CurrentTextIndex + 1
		);
		
		const FName TableId = CodexStringTable
		? CodexStringTable->GetStringTableId()
		: CodexStringTableId;
		
		DescriptionText->SetText(FText::FromStringTable(TableId, TextKeyString));
	}
	else	{
		DescriptionText->SetText(FText::GetEmpty());
	}
}
