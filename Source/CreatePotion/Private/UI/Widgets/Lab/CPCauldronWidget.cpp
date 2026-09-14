// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Lab/CPCauldronWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "Data/CPTagDefinitionTypes.h"
#include "Lab/Component/Interact/CPCauldronComponent.h"
#include "Settings/CPDTSettings.h"

void UCPCauldronWidget::SetCauldronComponent(UCPCauldronComponent* InCauldronComponent)
{
	UCPCauldronComponent* CurrentCauldronComponent = CauldronComponent.Get();
	if (CurrentCauldronComponent == InCauldronComponent){
		RefreshEffectTags();
		return;
	}
	
	if (CurrentCauldronComponent){
		CurrentCauldronComponent->OnEffectsChanged.RemoveDynamic(this, &UCPCauldronWidget::RefreshEffectTags);
	}
	
	CauldronComponent = InCauldronComponent;
	
	if (UCPCauldronComponent* NewCauldronComponent = CauldronComponent.Get()){
		NewCauldronComponent->OnEffectsChanged.AddUniqueDynamic(this, &UCPCauldronWidget::RefreshEffectTags);
	}
	
	RefreshEffectTags();
}

void UCPCauldronWidget::UnbindEvents()
{	
	if (UCPCauldronComponent* BoundCauldronComponent = CauldronComponent.Get()){
		CauldronComponent->OnEffectsChanged.RemoveDynamic(this, &UCPCauldronWidget::RefreshEffectTags);
	}
	
	CauldronComponent.Reset();
	Super::UnbindEvents();
}

void UCPCauldronWidget::ApplyDisplayStateColors(ECPInteractionDisplayState DisplayState)
{
	Super::ApplyDisplayStateColors(DisplayState);
	
	Txt_CurrentEffects->SetColorAndOpacity(TextBlock_InteractionPrompt->GetColorAndOpacity());
}

void UCPCauldronWidget::RefreshEffectTags()
{
	if (!GP_EffectTags) return;
	GP_EffectTags->ClearChildren();
	
	GP_EffectTags->SetColumnFill(0, 1.f);
	GP_EffectTags->SetColumnFill(1, 1.f);
	
	UCPCauldronComponent* BoundCauldronComponent = CauldronComponent.Get();
	if (!BoundCauldronComponent || !WidgetTree) return;
	
	constexpr int32 ColumnCount = 2;
	int32 DisplayIndex = 0;
	
	const TArray<FGameplayTag> EffectTags = BoundCauldronComponent->GetEffectTags();
	for (const FGameplayTag& Tag : EffectTags){
		if (!Tag.IsValid()) continue;
		
		UTextBlock* EffectTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (!EffectTextBlock) continue;
		
		EffectTextBlock->SetText(GetEffectDisplayName(Tag));
		EffectTextBlock->SetJustification(ETextJustify::Left);
		
		UGridSlot* GridSlot = GP_EffectTags->AddChildToGrid(
			EffectTextBlock, DisplayIndex / ColumnCount, DisplayIndex % ColumnCount);
		
		if (GridSlot){
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Center);
		}
		
		++DisplayIndex;
	}
	
	if (UVerticalBoxSlot* VBSlot = Cast<UVerticalBoxSlot>(GP_EffectTags->Slot)){
		VBSlot->SetPadding(FMargin(
			0.f, GP_EffectTags->GetChildrenCount() > 0 ? 10.f : 0.f, 0.f, GP_EffectTags->GetChildrenCount() > 0 ? 10.f : 0.f)
		);	
	}
}

FText UCPCauldronWidget::GetEffectDisplayName(const FGameplayTag& EffectTag) const
{
	if (!EffectTag.IsValid()) return FText::GetEmpty();
	
	const UCPDTSettings* DTSettings = GetDefault<UCPDTSettings>();
	UDataTable* TagDefinitionTable = DTSettings ? DTSettings->TagDefinitionTable.LoadSynchronous() : nullptr;
	if (!TagDefinitionTable) return FText::GetEmpty();
	
	TArray<FCPTagDefinitionRow*> Rows;
	TagDefinitionTable->GetAllRows<FCPTagDefinitionRow>(TEXT("CauldronWidgetTagDefinition"), Rows);
	
	for (const FCPTagDefinitionRow* Row : Rows){
		if (Row && Row->Tag == EffectTag){
			return Row->DisplayName;
		}
	}
	
	return FText::GetEmpty();
}
