// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "Data/CPForageableItemData.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "CPCodexItemGridWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCodexGridItemClicked, const FCPForageableCodexEntry&, CodexEntry);

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
	
	UFUNCTION()
	void HandleSlotClicked(const FCPForageableCodexEntry& SelectedCodexEntry);
	
	// --- Set 함수 ---
	void SetCodexEntries();
	
private:
	void GenerateSlots();

public:
	UPROPERTY(EditAnywhere, Category = "Codex | Grid")
	int32 GridColumn;
	
	UPROPERTY(EditAnywhere, Category = "Codex | Grid")
	int32 GridRow;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;
	
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
