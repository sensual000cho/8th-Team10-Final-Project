// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "Data/CPForageableItemData.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "CPCodexItemGridWidget.generated.h"

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
	
	void NativeConstruct() override;
	
	void BindEvents() override;
	void UnbindEvents() override;
	
	// --- Set 함수 ---
	UFUNCTION(BlueprintCallable, Category = "UCPCodexItemGridWidget")
	void SetItemData();
	
private:
	void GenerateSlots();

public:
	UPROPERTY(EditAnywhere, Category = "Codex | Grid")
	int32 GridColumn;
	
	UPROPERTY(EditAnywhere, Category = "Codex | Grid")
	int32 GridRow;
	
	UPROPERTY(EditAnywhere, Category = "Codex | Grid")
	TSubclassOf<class UCPCodexItemSlotWidget> CodexItemSlotWidgetClass;	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;
private:
	
	// --- 위젯 바인딩 ---
	UPROPERTY(meta = (BindWidget))	
	TObjectPtr<class UUniformGridPanel> Grid_ItemCodex;
	
	UPROPERTY()
	TArray<TObjectPtr<UCPCodexItemSlotWidget>> SlotWidgets;

};
