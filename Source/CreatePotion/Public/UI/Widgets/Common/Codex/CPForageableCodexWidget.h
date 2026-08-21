// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPForageableCodexWidget.generated.h"

class UCPCodexSubsystem;
class UTextBlock;
class UImage;
class UCPForageableItemData;

UCLASS()
class CREATEPOTION_API UCPForageableCodexWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()

public:
	UCPForageableCodexWidget(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category = "Codex")
	void ShowPreviousEntry();
	
	UFUNCTION(BlueprintCallable, Category = "Codex")
	void ShowNextEntry();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	UFUNCTION()	
	void RefreshCodex();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Codex")
	FName CodexStringTableId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UStringTable> CodexStringTable;
	
	UPROPERTY(BlueprintReadOnly, Category = "Codex")
	int32 CurrentEntryIndex;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CodexImage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TagText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TagCombinationText;
};
