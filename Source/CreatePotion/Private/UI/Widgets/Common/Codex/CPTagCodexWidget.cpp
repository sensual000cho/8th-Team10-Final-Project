// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Common/Codex/CPTagCodexWidget.h"

#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/CPTagDefinitionTypes.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "Settings/CPDTSettings.h"
#include "UI/Widgets/Common/Codex/CPCodexCombineRowWidget.h"

void UCPTagCodexWidget::NativeConstruct()
{
	UGameInstance* GameInstance = GetGameInstance();
	CodexSubsystem = GameInstance ? GameInstance->GetSubsystem<UCPCodexSubsystem>() : nullptr;
	Super::NativeConstruct();
	RebuildCombinations();
}

void UCPTagCodexWidget::BindEvents()
{
	Super::BindEvents();
	
	if (CodexSubsystem){
		CodexSubsystem->OnForageableCodexUpdated.AddUniqueDynamic(this, &UCPTagCodexWidget::RebuildCombinations);
	}
}

void UCPTagCodexWidget::UnbindEvents()
{
	if (CodexSubsystem){
		CodexSubsystem->OnForageableCodexUpdated.RemoveDynamic(this, &UCPTagCodexWidget::RebuildCombinations);
	}
	Super::UnbindEvents();
}

void UCPTagCodexWidget::RebuildCombinations()
{
	if (!VB_TagCombinations || !Txt_NoCombinations) return;
	VB_TagCombinations->ClearChildren();
	
	const UCPDTSettings* DTSettings = GetDefault<UCPDTSettings>();
	UDataTable* TagDefinitionTable = DTSettings ? DTSettings->TagDefinitionTable.LoadSynchronous() : nullptr;
	if (!TagDefinitionTable || !CodexSubsystem || !CombinationRowWidgetClass){
		Txt_NoCombinations->SetVisibility(ESlateVisibility::HitTestInvisible);
		return;
	}
	
	TArray<FCPTagDefinitionRow*> TagRows;
	TagDefinitionTable->GetAllRows<FCPTagDefinitionRow>("TagCodexDefinition", TagRows);
	
	TSet<FString> AddedCombinationKeys;
	for (const FCPTagDefinitionRow* TagRow : TagRows){
		if (!TagRow || !TagRow->Tag.IsValid()) continue;
		for (const FCPTagCombinationEntry& Combination : TagRow->Combinations){
			if (!Combination.OtherTag.IsValid() || !Combination.ResultTag.IsValid()) continue;
			
			const FString CombinationKey = MakeCombinationKey(TagRow->Tag, Combination.OtherTag, Combination.ResultTag);
			if (AddedCombinationKeys.Contains(CombinationKey)) continue;
			
			AddedCombinationKeys.Add(CombinationKey);
			UCPCodexCombineRowWidget* CombinationRow = CreateWidget<UCPCodexCombineRowWidget>(this, CombinationRowWidgetClass);
			
			if (!CombinationRow) continue;
			
			UCPForageableItemData* LeftItemData = CodexSubsystem->FindRegisteredItemByTag(TagRow->Tag);
			UCPForageableItemData* RightItemData = CodexSubsystem->FindRegisteredItemByTag(Combination.OtherTag);
			
			CombinationRow->SetCombinationData(
			MakeTagDisplayData(TagRow->Tag, TagRows),
			MakeTagDisplayData(Combination.OtherTag, TagRows),
			MakeTagDisplayData(Combination.ResultTag, TagRows));
			
			VB_TagCombinations->AddChildToVerticalBox(CombinationRow);
		}
	}
	Txt_NoCombinations->SetVisibility(VB_TagCombinations->GetChildrenCount() == 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

FString UCPTagCodexWidget::MakeCombinationKey(const FGameplayTag& FirstTag, const FGameplayTag& SecondTag,
	const FGameplayTag& ResultTag) const
{
	FString FirstTagName = FirstTag.ToString();
	FString SecondTagName = SecondTag.ToString();
	
	if (SecondTagName < FirstTagName){
		Swap(FirstTagName, SecondTagName);
	}
	
	return FString::Printf(TEXT("%s|%s|%s"), *FirstTagName, *SecondTagName, *ResultTag.ToString());
}

FCPTagCodexDisplayData UCPTagCodexWidget::MakeTagDisplayData(const FGameplayTag& Tag,
	const TArray<FCPTagDefinitionRow*>& TagRows) const
{
	FCPTagCodexDisplayData DisplayData;
	DisplayData.Tag = Tag;
	DisplayData.Text = FText::FromString(TEXT("???"));
	
	for (const FCPTagDefinitionRow* TagRow : TagRows){
		if (TagRow && TagRow->Tag == Tag){
			DisplayData.Text = TagRow->DisplayName;
			DisplayData.Color = TagRow->LiquidColor01;
			break;
		}
	}
	
	return DisplayData;
}
