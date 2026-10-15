// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Lab/CPCauldronWidget.h"

#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/CPTagDefinitionTypes.h"
#include "Lab/Component/Interact/CPCauldronComponent.h"
#include "Settings/CPDTSettings.h"
#include "UI/Widgets/Lab/CPCauldronEffectRowWidget.h"

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
	
	const bool bHasIngredients = !BoundCauldronComponent->GetIngredientInstance().IsEmpty();
	Txt_CurrentEffects->SetVisibility(bHasIngredients ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	
	if (!EffectRowWidgetClass) return;
	
	const UCPDTSettings* DTSettings = GetDefault<UCPDTSettings>();
	UDataTable* TagDefinitionTable = DTSettings ? DTSettings->TagDefinitionTable.LoadSynchronous() : nullptr;
	
	// 효과 VB에 추가 + 가운데 정렬
	const TArray<FGameplayTag> EffectTags = BoundCauldronComponent->GetEffectTags();
	for (const FGameplayTag& Tag : EffectTags){
		if (!Tag.IsValid()) continue;
		
		const FCPTagDefinitionRow* TagDefinition = FCPTagDefinitionLookup::FindRow(TagDefinitionTable, Tag);
		if (!TagDefinition) continue;
		
		UCPCauldronEffectRowWidget* EffectRow = CreateWidget<UCPCauldronEffectRowWidget>(this, EffectRowWidgetClass);
		if (!EffectRow) continue;
		
		EffectRow->SetEffectData(TagDefinition->DisplayName, TagDefinition->LiquidColor01);
		VB_EffectTags->AddChildToVerticalBox(EffectRow);
	}
}
