// CPPotionActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "CPPotionActor.generated.h"

UCLASS()
class CREATEPOTION_API ACPPotionActor : public ACPThrowablePropBase
{
	GENERATED_BODY()
	
public:
	virtual FText GetInteractionPrompt_Implementation() override;
};
