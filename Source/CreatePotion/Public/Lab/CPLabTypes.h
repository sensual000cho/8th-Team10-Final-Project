// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CPLabTypes.generated.h"


class UCPForageableItemData;


UENUM(BlueprintType)
enum class ECPLabSessionPhase : uint8
{
    WaitingForBell,
    Request,
    Preparing,
    Processing,
    Result,
};

USTRUCT(BlueprintType)
struct FCPLabRequest
{
    GENERATED_BODY()

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Lab|Request")
    //ID를 이용해 리퀘스트 불러오는 방식으로
    FName RequestId = NAME_None;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Lab|Request")
    FText DisplayText;

    bool IsValid() const
    {
        return !RequestId.IsNone() &&
            !DisplayText.IsEmpty();
    }
};

USTRUCT(BlueprintType)
struct FCPLabIngredientInstance
{
    GENERATED_BODY()

    // 이 작업 재료가 어떤 원본 재료에서 만들어졌는지
    UPROPERTY(BlueprintReadOnly, Category = "Lab|Ingredient")
    TObjectPtr<UCPForageableItemData> SourceItemData = nullptr;

    // 가공에 따라 변경되는 현재 효과값
    UPROPERTY(BlueprintReadOnly, Category = "Lab|Ingredient")
    TMap<FGameplayTag, int32> CurrentEffects;

    bool IsValid() const
    {
        return SourceItemData != nullptr;
    }

    int32 GetEffectValue(const FGameplayTag& EffectTag) const
    {
        return CurrentEffects.FindRef(EffectTag);
    }
};