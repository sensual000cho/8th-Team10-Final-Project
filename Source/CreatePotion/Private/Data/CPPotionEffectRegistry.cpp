#include "Data/CPPotionEffectRegistry.h"

#include "GameFramework/Actor.h"

bool UCPPotionEffectRegistry::TryApplyPotionEffects(const FCPPotionImpactContext& Context) const
{
	if (!IsValid(Context.TargetActor))
	{
		return false;
	}

	bool bAnyEffectApplied = false;

	for (const FGameplayTag& EffectTag : Context.EffectTags)
	{
		const TSubclassOf<UCPPotionEffectHandler>* HandlerClass = EffectHandlers.Find(EffectTag);

		// 미등록 태그는 해당 Registry에서 지원하지 않음
		if (!HandlerClass)
		{
			continue;
		}

		UClass* HandlerUClass = HandlerClass->Get();

		// None 또는 Abstract Handler는 명시적인 미적용으로 처리
		if (!HandlerUClass || HandlerUClass->HasAnyClassFlags(CLASS_Abstract))
		{
			continue;
		}

		UCPPotionEffectHandler* Handler = NewObject<UCPPotionEffectHandler>(Context.TargetActor.Get(), HandlerUClass);

		if (!IsValid(Handler))
		{
			continue;
		}

		if (Handler->TryApplyPotionEffect(EffectTag, Context))
		{
			bAnyEffectApplied = true;
		}
	}

	return bAnyEffectApplied;
}
