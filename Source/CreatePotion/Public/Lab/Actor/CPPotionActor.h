// CPPotionActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "CPPotionActor.generated.h"

class UCPPotionImpactComponent;

UCLASS()
class CREATEPOTION_API ACPPotionActor : public ACPThrowablePropBase
{
	GENERATED_BODY()

public:
	ACPPotionActor();

	virtual FText GetInteractionPrompt_Implementation() override;

	// 완성된 포션의 태그를 저장하고 PotionImpactComponent에도 전달.
	UFUNCTION(BlueprintCallable, Category = "Potion")
	void InitializePotionEffects(const TArray<FGameplayTag>& InEffectTags);

	UFUNCTION(BlueprintPure, Category = "Potion")
	const TArray<FGameplayTag>& GetPotionEffectTags() const;

	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	UCPPotionImpactComponent* GetPotionImpactComponent() const;

protected:
	virtual void HandleThrowStarted(AActor* Thrower) override;
	virtual void HandleThrownImpact(AActor* OtherActor, const FHitResult& HitResult) override;

	void TriggerPotionExplosion(const FHitResult& HitResult);
	
	// BP에서 Effect 관련 나이아가라, 사운드 등 구현
	UFUNCTION(BlueprintImplementableEvent, Category = "Potion|Impact", meta = (DisplayName = "On Potion Exploded"))
	void K2_OnPotionExploded(const FHitResult& HitResult);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Impact", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCPPotionImpactComponent> PotionImpactComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion", meta = (AllowPrivateAccess = "true"))
	TArray<FGameplayTag> PotionEffectTags;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Impact", meta = (AllowPrivateAccess = "true"))
	bool bExplosionTriggered = false;
};
