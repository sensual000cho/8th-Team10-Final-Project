// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "UI/WIdgets/Common/Codex/CPCodexItemGridWidget.h"
#include "CPCodexItemListWidget.generated.h"

struct FCPForageableCodexEntry;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCodexItemListWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	void BindEvents() override;

private:
	void HandleSlotClicked(const FCPForageableCodexEntry& CodexEntry);

public:
	
private:
	// --- 위젯 바인딩 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCPCodexItemGridWidget> CodexItemGrid;
};
