// CPPotionActor.cpp

#include "Lab/Actor/CPPotionActor.h"

FText ACPPotionActor::GetInteractionPrompt_Implementation()
{
	return FText::FromString(TEXT("포션 들기"));
}


