// CPItemInstanceTypes.cpp

#include "Types/CPItemInstanceTypes.h"
#include "Settings/CPPotionVisualSettings.h"

FLinearColor FCPItemInstance::GetLiquidColor() const
{
	const UCPPotionVisualSettings* Settings = GetDefault<UCPPotionVisualSettings>();

	if (CurrentEffects.Num() == 0)
	{
		return Settings->DefaultLiquidColor;
	}

	FLinearColor Blended = FLinearColor::Black;
	int32 MatchedCount = 0;

	for (const FGameplayTag& Tag : CurrentEffects)
	{
		if (const FLinearColor* Found = Settings->EffectColors.Find(Tag))
		{
			Blended += *Found;
			MatchedCount++;
		}
	}

	if (MatchedCount == 0)
	{
		return Settings->DefaultLiquidColor;
	}

	return Blended / static_cast<float>(MatchedCount); // 여러 효과면 색을 평균 혼합
}
