// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CPLabTypes.generated.h"

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