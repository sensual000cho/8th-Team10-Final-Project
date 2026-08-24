// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "UI/WIdgets/Common/Codex/CPCodexItemGridWidget.h"
#include "CPCodexItemListWidget.generated.h"

struct FCPForageableCodexEntry;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCodexItemListWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;
	
	void BindEvents() override;
	
private:
	UFUNCTION()
	void HandleCodexItemSelected(UCPForageableItemData* ItemData);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;
	
private:
	
	
	// --- 위젯 바인딩 ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCPCodexItemGridWidget> CodexItemGrid;
};
