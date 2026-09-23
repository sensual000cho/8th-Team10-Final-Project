// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPTagCodexWidget.generated.h"

class UCPCodexSubsystem;
class UTextBlock;
class UVerticalBox;
class UCPCodexCombineRowWidget;
struct FCPTagCodexDisplayData;
struct FCPTagDefinitionRow;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPTagCodexWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;
	
private:
	UFUNCTION()
	void RebuildCombinations();
	
	FString MakeCombinationKey(const FGameplayTag& FirstTag, const FGameplayTag& SecondTag, const FGameplayTag& ResultTag) const;
	
	FCPTagCodexDisplayData MakeTagDisplayData(
		const FGameplayTag& Tag,
		const TArray<FCPTagDefinitionRow*>& TagRows) const;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Codex|Tag")
	TSubclassOf<UCPCodexCombineRowWidget> CombinationRowWidgetClass;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_TagCombinations;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_NoCombinations;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;
};

