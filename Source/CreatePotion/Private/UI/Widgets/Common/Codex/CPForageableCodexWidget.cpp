// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPForageableCodexWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/CPForageableItemData.h"
#include "Data/CPTagDefinitionTypes.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "Internationalization/StringTable.h"
#include "Settings/CPDTSettings.h"
#include "UI/Widgets/Common/Codex/CPCodexCombineRowWidget.h"

UCPForageableCodexWidget::UCPForageableCodexWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCPForageableCodexWidget::SetCodexEntry(const FCPForageableCodexEntry& InCodexEntry)
{
	SetForageableItem(InCodexEntry.Entry);
}

void UCPForageableCodexWidget::SetForageableItem(UCPForageableItemData* InItemData)
{
	CurrentItemData = InItemData;
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
	if (!CurrentItemData || !CodexSubsystem) return;
	
	FCPForageableCodexEntry CurrentCodexEntry;
	if (!CodexSubsystem->GetForageableEntry(CurrentItemData, &CurrentCodexEntry)) return;
	
	// 전달받은 FCPForageableCodexEntry로 도감 초기화
	UTexture2D* CodexImageTexture = CurrentItemData->CodexImage.LoadSynchronous();
	Img_CodexImage->SetBrushFromTexture(CodexImageTexture);
	
	Txt_NameText->SetText(CurrentItemData->DisplayName);
	
	const TArray<FGameplayTag> Tags = CurrentItemData->TagAxes;
	Txt_TagText->SetText(BuildTagText(Tags));
	
	FLinearColor TagBorderColor = FLinearColor::Transparent;
	BuildTagCombination(CurrentItemData->TagAxes);
	
	// 현재 태그 Border 색상 설정
	if (!Tags.IsEmpty() || Tags[0].IsValid()){
		const FCPTagDefinitionRow* TagRow = nullptr;
		if (FindTagDefinitionRow(Tags[0], TagRow) && TagRow){
			TagBorderColor = TagRow->LiquidColor01;
			TagBorderColor.A *= TagColorOpacity;
		}
	}
	
	Border_Tag->SetBrushColor(TagBorderColor);
	Txt_HarvestCountText->SetText(FText::Format(
		FText::FromString(TEXT("{0} 회")), FText::AsNumber(CurrentCodexEntry.HarvestCount)));
	
	// ST에서 사용할 키 생성 후 가져오기
	if (!CurrentItemData->CodexTextKeys.IsEmpty())	{
		const FName CodexKeyPrefix = CurrentItemData->CodexTextKeys[0];

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
		return FText::FromString(TEXT("없음"));
	}
	
	return GetTagDisplayText(Tags[0]);
}

void UCPForageableCodexWidget::BuildTagCombination(const TArray<FGameplayTag>& Tags)
{
	VB_TagCombinations->ClearChildren();
	
	if (Tags.IsEmpty() || !Tags[0].IsValid() || !CombinationRowWidgetClass){
		Txt_TagCombinationText->SetVisibility(ESlateVisibility::HitTestInvisible);
		return;
	}
	
	const FGameplayTag& Tag = Tags[0];
	
	// 태그에 대응하는 DT Row가 있는지 확인
	const FCPTagDefinitionRow* TagRow = nullptr;
	if (!FindTagDefinitionRow(Tag, TagRow) || !TagRow){
		Txt_TagCombinationText->SetVisibility(ESlateVisibility::HitTestInvisible);
		return;
	}
	
	// DT에 정의된 조합 가능 정보를 사용해서 문자열을 표시
	for (const FCPTagCombinationEntry& Combination : TagRow->Combinations){
		if (!Combination.OtherTag.IsValid() || !Combination.ResultTag.IsValid()) continue;
		
		UCPCodexCombineRowWidget* CombinationRow = CreateWidget<UCPCodexCombineRowWidget>(this, CombinationRowWidgetClass);
		if (!CombinationRow) continue;

		TagColorOpacity = CombinationRow->GetTagColorOpacity();
		CombinationRow->SetCombinationData(MakeTagDisplayData(Tag), MakeTagDisplayData(Combination.OtherTag), 
			MakeTagDisplayData(Combination.ResultTag), false);
		
		VB_TagCombinations->AddChildToVerticalBox(CombinationRow);
	}
	
	Txt_TagCombinationText->SetVisibility(VB_TagCombinations->GetChildrenCount() == 0 
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

FCPTagCodexDisplayData UCPForageableCodexWidget::MakeTagDisplayData(const FGameplayTag& Tag) const
{
	FCPTagCodexDisplayData DisplayData;
	DisplayData.Tag = Tag;
	DisplayData.Text = FText::FromString(TEXT("???"));
	
	const FCPTagDefinitionRow* TagRow = nullptr;
	if (FindTagDefinitionRow(Tag, TagRow) && TagRow) {
		DisplayData.Text = TagRow->DisplayName;
		DisplayData.Color = TagRow->LiquidColor01;
	}

	return DisplayData;
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
