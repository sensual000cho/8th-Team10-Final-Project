#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "GameplayTagContainer.h"
#include "CPPotionImpactComponent.generated.h"

class AActor;
class APawn;
class UAbilitySystemComponent;

// 포션의 첫 충돌 지점에서 효과 범위를 한 번 검사하고 대상 ASC에 효과 Gameplay Event를 전달한다.
// Potion Actor 연결 순서: SetPotionEffectTags -> EnableImpactProcessing -> TryTriggerPotionImpact.
UCLASS(ClassGroup=(Potion), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPPotionImpactComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPPotionImpactComponent();

	// 포션이 실제로 투척된 뒤 충돌 처리를 허용한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	bool EnableImpactProcessing(APawn* InInstigator);

	// Impact 처리를 중단한다. 이미 발생한 Impact 상태는 되돌리지 않는다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	void DisableImpactProcessing();

	// 포션 Actor의 첫 유효 Hit를 잠그고 충돌 지점에서 효과 범위를 한 번 검사한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	bool TryTriggerPotionImpact(const FHitResult& HitResult, const TArray<FGameplayTag>& PotionEffectTags);

	// 첫 충돌 위치에서 Target ASC를 찾을 효과 범위의 반지름이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Impact", meta = (ClampMin = "0.0", Units = "cm"))
	float EffectRadius = 250.0f;

	// 효과 범위 검사에 포함할 Collision Object Type 목록이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Impact")
	TArray<TEnumAsByte<EObjectTypeQuery>> EffectObjectTypes;

	//충돌처리 가능한지 묻는 함수
	UFUNCTION(BlueprintPure, Category = "Potion|Impact|State")
	bool IsImpactProcessingEnabled() const;

	// 이 포션에서 첫 Impact가 이미 발생했는지 확인한다.
	UFUNCTION(BlueprintPure, Category = "Potion|Impact|State")
	bool HasPotionImpactTriggered() const;

private:
	// 충돌 했을 때 해당 충돌을 "첫 충돌" 로 규정하고 실제 대상 검사 시행 시작
	bool TryCommitPotionImpact(const FHitResult& HitResult, const TArray<FGameplayTag>& PotionEffectTags);

	// Impact 위치에서 Sphere Overlap을 한 번 실행하고 찾은 대상 ASC에 효과 Event를 전달한다.
	void ResolvePotionEffectArea(const FHitResult& HitResult, const TArray<FGameplayTag>& PotionEffectTags);

	// Potion.Effect 태그를 그대로 Gameplay Event로 사용해 Target ASC로 전달한다
	void TryDispatchPotionEffectToTarget(AActor* TargetActor, FGameplayTag EffectTag, const FHitResult& HitResult, UAbilitySystemComponent* SourceAbilitySystem, UAbilitySystemComponent* TargetAbilitySystem);

	// 투척자 ASC를 찾는 함수. 이외에 더미 찾는 부분도 있는데 테스트 후 삭제 예정.
	UAbilitySystemComponent* ResolveSourceAbilitySystem() const;

	// Gameplay Event와 Effect Context에 전달할 실제 투척 Pawn.
	UPROPERTY()
	TObjectPtr<APawn> ImpactInstigator = nullptr;

	// Impact 처리가 활성화된 뒤, 첫 Impact가 발생하거나 비활성화될 때까지 true.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bImpactProcessingEnabled = false;

	// 반복 Hit가 새로운 범위 검사를 만들지 않도록 첫 Impact를 잠근 상태.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bImpactTriggered = false;
};
