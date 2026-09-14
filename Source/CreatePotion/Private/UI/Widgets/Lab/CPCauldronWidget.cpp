// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Lab/CPCauldronWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
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
	if (!VB_EffectTags) return;
	VB_EffectTags->ClearChildren();
	
	UCPCauldronComponent* BoundCauldronComponent = CauldronComponent.Get();
	if (!BoundCauldronComponent || !WidgetTree) return;
	
	const TArray<FGameplayTag> EffectTags = BoundCauldronComponent->GetEffectTags();
	for (const FGameplayTag& Tag : EffectTags){
		if (!Tag.IsValid()) continue;
		
		UTextBlock* EffectTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (!EffectTextBlock) continue;
		
		EffectTextBlock->SetText(GetEffectDisplayName(Tag));
		EffectTextBlock->SetJustification(ETextJustify::Center);
		VB_EffectTags->AddChildToVerticalBox(EffectTextBlock);
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
