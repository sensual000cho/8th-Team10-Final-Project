// CPPotionActor.h

#pragma once

#include "CoreMinimal.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "CPPotionActor.generated.h"

struct FCPTagDefinitionRow;
class UCPPotionImpactComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

struct FCPPotionVisualColors
{
	FLinearColor LiquidColor01;
	FLinearColor LiquidColor02;
	FLinearColor SurfaceColor01;
};

UCLASS()
class CREATEPOTION_API ACPPotionActor : public ACPThrowablePropBase
{
	GENERATED_BODY()

public:
	ACPPotionActor();

	virtual void Tick(float DeltaTime) override;
	virtual FName GetInteractionName_Implementation() override;
	virtual void InitializeFromItemData(UCPForageableItemData* ItemData, const TArray<FGameplayTag>& EffectTags = TArray<FGameplayTag>()) override;
	
	// Spawn Impulse 적용
	void ApplySpawnImpulse(const FVector& SpawnImpulse);

protected:
	virtual void HandleThrowStarted(AActor* Thrower) override;
	virtual void HandleThrownImpact(AActor* OtherActor, const FHitResult& HitResult) override;

	void TriggerPotionExplosion(const FHitResult& HitResult);
	
	// 포션 시각 요소 초기화
	void ApplyPotionVisual();
	
	// 포션 색상 갱신
	void UpdatePotionVisual(float DeltaTime);
	
	// 계산된 포션 색상 적용
	void ApplyLiquidMaterialColors() const;
	
	// BP에서 Effect 관련 나이아가라, 사운드 등 구현
	UFUNCTION(BlueprintImplementableEvent, Category = "Potion|Impact", meta = (DisplayName = "On Potion Exploded"))
	void K2_OnPotionExploded(const FHitResult& HitResult);

private:
	// 포션 Impact Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Impact", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCPPotionImpactComponent> PotionImpactComponent;
	
	// 포션 Visual(NS, MI)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|visual", meta = (AllowPrivateAccess = "true"))
	FComponentReference LiquidNiagaraComponentReference;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> LiquidMaterial;
	
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicLiquidMaterial;

	// 포션 색상 변경 값
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|visual", meta = (AllowPrivateAccess = "true"))
	float ColorTransitionDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|visual", meta = (AllowPrivateAccess = "true"))
	float ColorHoldDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|visual", meta = (AllowPrivateAccess = "true"))
	float TransitionNeutralizeStrength;

	FCPPotionVisualColors CurrentVisualColors;
	
	TArray<FCPPotionVisualColors> VisualColors;

	int32 ColorIndex;
	float RemainingTime;
	bool bTransitioning;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Impact", meta = (AllowPrivateAccess = "true"))
	bool bExplosionTriggered;
};
