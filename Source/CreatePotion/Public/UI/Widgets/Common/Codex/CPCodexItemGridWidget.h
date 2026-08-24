// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "Data/CPForageableItemData.h"
#include "CPCodexItemGridWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCodexGridItemClicked, UCPForageableItemData*, ItemData);

class UCPCodexItemSlotWidget;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCodexItemGridWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	UCPCodexItemGridWidget();
	
	UFUNCTION()
	void HandleSlotClicked(UCPForageableItemData* ItemData);
	
	void InitGrid(const TArray<class UCPForageableItemData*>& ItemDatas);

	
private:
	void GenerateSlots();

public:
	UPROPERTY(EditAnywhere, Category = "Codex | Grid")
	int32 GridColumn;
	
	UPROPERTY(EditAnywhere, Category = "Codex | Grid")
	int32 GridRow;
	
	UPROPERTY(EditAnywhere, Category = "Codex | Grid")
	TSubclassOf<class UCPCodexItemSlotWidget> CodexItemSlotWidgetClass;	
	
	UPROPERTY(BlueprintAssignable, Category = "Codex | Grid")
	FOnCodexGridItemClicked OnCodexGridItemClicked;
private:
	// --- 위젯 바인딩 ---
	UPROPERTY(meta = (BindWidget))	
	TObjectPtr<class UUniformGridPanel> Grid_ItemCodex;
	
	UPROPERTY()
	TArray<TObjectPtr<UCPCodexItemSlotWidget>> SlotWidgets;

};
