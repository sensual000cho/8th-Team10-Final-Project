// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPForageableCodexWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CPForageableItemData.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "Internationalization/StringTable.h"

UCPForageableCodexWidget::UCPForageableCodexWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CodexStringTableId(TEXT("ST_ForageableCodex"))
	, CurrentEntryIndex(0)
{
}

void UCPForageableCodexWidget::ShowPreviousEntry()
{
	if (!CodexSubsystem) return;
	
	const TArray<FCPForageableCodexEntry> Entries = CodexSubsystem->GetForageableCodexEntries();
	if (Entries.Num() <= 0) return;
	
	CurrentEntryIndex--;
	if (CurrentEntryIndex < 0) CurrentEntryIndex += Entries.Num();
	
	RefreshCodex();
}

void UCPForageableCodexWidget::ShowNextEntry()
{
	if (!CodexSubsystem) return;
	
	const TArray<FCPForageableCodexEntry> Entries = CodexSubsystem->GetForageableCodexEntries();
	if (Entries.Num() <= 0) return;
	
	CurrentEntryIndex = (CurrentEntryIndex+1) % Entries.Num();
	
	RefreshCodex();
}

void UCPForageableCodexWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CodexSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCPCodexSubsystem>() : nullptr;
	if (CodexSubsystem){
		CodexSubsystem->OnForageableCodexUpdated.AddDynamic(this, &UCPForageableCodexWidget::RefreshCodex);	
	}
	
	RefreshCodex();
}

void UCPForageableCodexWidget::NativeDestruct()
{
	if (CodexSubsystem){
		CodexSubsystem->OnForageableCodexUpdated.RemoveDynamic(this, &UCPForageableCodexWidget::RefreshCodex);
	}
	
	Super::NativeDestruct();
}

void UCPForageableCodexWidget::RefreshCodex()
{
	const TArray<FCPForageableCodexEntry> CodexEntries = 
		CodexSubsystem ? CodexSubsystem->GetForageableCodexEntries() : TArray<FCPForageableCodexEntry>();
	
	// 해당 번호의 Index가 유효하지 않은 경우
	if (!CodexEntries.IsValidIndex(CurrentEntryIndex)){
		if (CodexImage){
			CodexImage->SetBrushFromTexture(nullptr);
		}
		
		if (NameText){
			NameText->SetText(FText::GetEmpty());
		}
		
		if (DescriptionText){
			DescriptionText->SetText(FText::GetEmpty());
		}
		
		if (TagText){
			TagText->SetText(FText::GetEmpty());
		}
		return;
	}
	
	const FCPForageableCodexEntry& CurrentCodexEntry = CodexEntries[CurrentEntryIndex];
	const UCPForageableItemData* CurrentEntry = CurrentCodexEntry.Entry;
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
			TEXT("%s.%02d"), *CodexKeyPrefix.ToString(), CurrentCodexEntry.Level);
		
		const FName TableId = CodexStringTable ? CodexStringTable->GetStringTableId() : CodexStringTableId;
		
		DescriptionText->SetText(FText::FromStringTable(TableId, TextKeyString));
	}
	else {
		DescriptionText->SetText(FText::GetEmpty());
	}
}
