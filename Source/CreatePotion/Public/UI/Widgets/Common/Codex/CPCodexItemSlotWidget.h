// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPCodexItemSlotWidget.generated.h"

class UCPForageableItemData;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPCodexItemSlotWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UCPForageableItemData> ItemData;
};

