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