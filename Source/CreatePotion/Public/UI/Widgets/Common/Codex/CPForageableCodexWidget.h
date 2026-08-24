// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPForageableCodexWidget.generated.h"

struct FCPForageableCodexEntry;
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
	
	// 해당 WBP가 표시할 내용 설정
	UFUNCTION(BlueprintCallable, Category = "Codex")
	void SetCodexEntry(const FCPForageableCodexEntry& InCodexEntry);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	// 도감 갱신
	UFUNCTION()	
	void RefreshCodex();
	
	// 태그 텍스트 설정
	FText BuildTagText(const TArray<FGameplayTag>& Tags) const;
	
	// 조합 가능 텍스트 설정
	FText BuildTagCombinationText(const TArray<FGameplayTag>& Tags) const;
	
	// DisplayName을 사용해 태그 텍스트 설정
	FText GetTagDisplayText(const FGameplayTag& Tag) const;
	
	// 지정한 태그에 대응하는 Row을 탐색
	bool FindTagDefinitionRow(const FGameplayTag& Tag, const struct FCPTagDefinitionRow*& OutRow) const;
	
protected:
	// UMG
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_CodexImage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_NameText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_DescriptionText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_TagText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_TagCombinationText;
	
protected:
	// 도감 데이터 서브시스템
	UPROPERTY(BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPCodexSubsystem> CodexSubsystem;
	
	// 도감 설명 텍스트 String Table
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Codex")
    TObjectPtr<UStringTable> CodexStringTable;
	
	// 현재 표시 대상 채집물 도감 Entry
	UPROPERTY(BlueprintReadOnly, Category = "Codex")
	FCPForageableCodexEntry CurrentCodexEntry;
};
