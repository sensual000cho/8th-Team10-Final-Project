// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPForageableCodexWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CPForageableItemData.h"
#include "Data/CPTagDefinitionTypes.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "Internationalization/StringTable.h"
#include "Settings/CPDTSettings.h"

UCPForageableCodexWidget::UCPForageableCodexWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCPForageableCodexWidget::SetCodexEntry(const FCPForageableCodexEntry& InCodexEntry)
{
	CurrentCodexEntry = InCodexEntry;
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
	// 해당 번호의 Index가 유효하지 않은 경우
	const UCPForageableItemData* CurrentEntry = CurrentCodexEntry.Entry;
	if (!CurrentEntry) return;
	
	// 전달받은 FCPForageableCodexEntry로 도감 초기화
	UTexture2D* CodexImageTexture = CurrentEntry->CodexImage.LoadSynchronous();
	Img_CodexImage->SetBrushFromTexture(CodexImageTexture);
	
	Txt_NameText->SetText(CurrentEntry->DisplayName);
	Txt_TagText->SetText(BuildTagText(CurrentEntry->TagAxes));
	Txt_TagCombinationText->SetText(BuildTagCombinationText(CurrentEntry->TagAxes));
	
	// ST에서 사용할 키 생성 후 가져오기
	if (!CurrentEntry->CodexTextKeys.IsEmpty())	{
		const FName CodexKeyPrefix = CurrentEntry->CodexTextKeys[0];

		const FString TextKeyString = FString::Printf(
			TEXT("%s.%02d"), *CodexKeyPrefix.ToString(), CurrentCodexEntry.Level);
		
		const FName TableId = CodexStringTable->GetStringTableId();
		Txt_DescriptionText->SetText(FText::FromStringTable(TableId, TextKeyString));
	}
	else {
		Txt_DescriptionText->SetText(FText::FromString(TEXT("조사가 필요합니다.")));
	}
}

FText UCPForageableCodexWidget::BuildTagText(const TArray<FGameplayTag>& Tags) const
{
	if (Tags.IsEmpty() || !Tags[0].IsValid()){
		return FText::FromString(TEXT("조사가 필요합니다."));
	}
	
	return GetTagDisplayText(Tags[0]);
}

FText UCPForageableCodexWidget::BuildTagCombinationText(const TArray<FGameplayTag>& Tags) const
{
	if (Tags.IsEmpty() || !Tags[0].IsValid()){
		return FText::FromString(TEXT("조사가 필요합니다."));
	}
	
	const FGameplayTag& Tag = Tags[0];
	
	// 태그에 대응하는 DT Row가 있는지 확인
	const FCPTagDefinitionRow* TagRow = nullptr;
	if (!FindTagDefinitionRow(Tag, TagRow) || !TagRow){
		return FText::FromString(TEXT("조사가 필요합니다."));
	}
	
	// DT에 정의된 조합 가능 정보를 사용해서 문자열을 표시
	TArray<FString> CombinationTexts;
	for (const FCPTagCombinationEntry& Combination : TagRow->Combinations){
		if (!Combination.OtherTag.IsValid() || !Combination.ResultTag.IsValid()) continue;
		
		CombinationTexts.Add(FString::Printf(TEXT("%s + %s = %s"),
			*GetTagDisplayText(Tag).ToString(),
			*GetTagDisplayText(Combination.OtherTag).ToString(),
			*GetTagDisplayText(Combination.ResultTag).ToString()));
	}
	
	return CombinationTexts.IsEmpty() ? 
		FText::FromString(TEXT("없음")) : FText::FromString(FString::Join(CombinationTexts, TEXT("\n")));
}

FText UCPForageableCodexWidget::GetTagDisplayText(const FGameplayTag& Tag) const
{
	// DisplayName이 지정되어 있으면 사용
	const FCPTagDefinitionRow* TagRow = nullptr;
	if (FindTagDefinitionRow(Tag, TagRow) && TagRow && !TagRow->DisplayName.IsEmpty()) {
		return TagRow->DisplayName;
	}
	
	// Tag의 마지막 단어 사용
	FString TagName = Tag.ToString();
	int32 LastSeparatorIndex = INDEX_NONE;
	if (TagName.FindLastChar(TEXT('.'), LastSeparatorIndex)){
		TagName.RightChopInline(LastSeparatorIndex + 1);
	}
	
	return FText::FromString(TagName);
}

bool UCPForageableCodexWidget::FindTagDefinitionRow(
	const FGameplayTag& Tag, const struct FCPTagDefinitionRow*& OutRow) const
{
	OutRow = nullptr;
	if (!Tag.IsValid()) return false;
	
	const UCPDTSettings* DTSettings = GetDefault<UCPDTSettings>();
	if (!DTSettings) return false;
	
	UDataTable* TagDefinitionTable = DTSettings->TagDefinitionTable.LoadSynchronous();
	if (!TagDefinitionTable) return false;
	
	TArray<FCPTagDefinitionRow*> Rows;
	TagDefinitionTable->GetAllRows<FCPTagDefinitionRow>(TEXT("ForageableCodexTagDefinition"), Rows);
	
	// DT의 각 Row을 순회하며 현재 Tag와 일치하는 정의를 찾는다
	for (const FCPTagDefinitionRow* Row : Rows){
		if (Row && Row->Tag == Tag) {
			OutRow = Row;
			return true;
		}
	}
	
	return false;
}
