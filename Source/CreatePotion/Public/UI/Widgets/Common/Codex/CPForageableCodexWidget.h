// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CPForageableCodexWidget.generated.h"

class UTextBlock;
class UImage;
class UCPForageableItemData;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPForageableCodexWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCPForageableCodexWidget(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category = "Codex")
	void ShowPreviousEntry();
	
	UFUNCTION(BlueprintCallable, Category = "Codex")
	void ShowNextEntry();
	
	UFUNCTION(BlueprintCallable, Category = "Codex")
	void SetTextIndex(int32 Index);
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Codex")
	TArray<TObjectPtr<UCPForageableItemData>> Entries;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Codex")
	FName CodexStringTableId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UStringTable> CodexStringTable;
	
	UPROPERTY(BlueprintReadOnly, Category = "Codex")
	int32 CurrentEntryIndex;
	
	UPROPERTY(BlueprintReadOnly, Category = "Codex")
	int32 CurrentTextIndex;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CodexImage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;
	
private:
	void RefreshCodex();
};
