// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPNPCCodexGridWidget.generated.h"

class UCPCodexSubsystem;
class UCPNPCSlotWidget;
class UUniformGridPanel;

UCLASS()
class CREATEPOTION_API UCPNPCCodexGridWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	UCPNPCCodexGridWidget();

	virtual void NativeConstruct() override;
	virtual void BindEvents() override;
	virtual void UnbindEvents() override;

	UFUNCTION(BlueprintCallable, Category = "Codex|NPC")
	void SetNPCData();

private:
	void GenerateSlots();

public:
	UPROPERTY(EditAnywhere, Category = "Codex|NPC|Grid")
	int32 GridColumn;

	UPROPERTY(EditAnywhere, Category = "Codex|NPC|Grid")
	int32 GridRow;

	UPROPERTY(EditAnywhere, Category = "Codex|NPC|Grid")
	TSubclassOf<UCPNPCSlotWidget> NPCSlotWidgetClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> Grid_NPCCodex;

	UPROPERTY()
	TArray<TObjectPtr<UCPNPCSlotWidget>> SlotWidgets;
	
};
