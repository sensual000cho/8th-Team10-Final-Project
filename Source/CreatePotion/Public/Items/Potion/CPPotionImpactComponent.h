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

UENUM(BlueprintType)
enum class ECPPotionEffectDispatchResult : uint8
{
	AbilityActivated,
	MissingSourceAbilitySystem,
	MissingTargetAbilitySystem,
	InvalidEffectTag,
	NoAbilityActivated
};

// Impact가 각 대상 ASC에 효과 Gameplay Event를 전달한 결과를 기록한다.
USTRUCT(BlueprintType)
struct CREATEPOTION_API FCPPotionEffectDispatchResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Potion|GAS")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Potion|GAS")
	FGameplayTag EffectTag;

	UPROPERTY(BlueprintReadOnly, Category = "Potion|GAS")
	ECPPotionEffectDispatchResult Result = ECPPotionEffectDispatchResult::NoAbilityActivated;
};

// 포션의 첫 충돌 지점에서 효과 범위를 한 번 검사하고 대상 ASC에 효과 Gameplay Event를 전달한다.
UCLASS(ClassGroup=(Potion), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPPotionImpactComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPPotionImpactComponent();

	// 포션이 실제로 투척된 뒤 충돌 처리를 허용한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	bool EnableImpactProcessing(APawn* InInstigator);

	// 포션 Actor가 보유한 현재 효과 태그를 Impact 경계에 전달한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	void SetPotionEffectTags(const TArray<FGameplayTag>& InEffectTags);

	// Impact 처리를 중단한다. 이미 발생한 Impact 상태는 되돌리지 않는다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	void DisableImpactProcessing();

	// 포션 Actor의 첫 유효 Hit를 잠그고 충돌 지점에서 효과 범위를 한 번 검사한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	bool TryTriggerPotionImpact(const FHitResult& HitResult);

	// 물리 Hit 없이 지정한 위치에서 동일한 1회 효과 범위 검사를 실행한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	bool TryTriggerPotionImpactAtLocation(FVector ImpactPoint, FVector ImpactNormal);

	// 첫 충돌 위치에서 Target ASC를 찾을 효과 범위의 반지름이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Impact", meta = (ClampMin = "0.0", Units = "cm"))
	float EffectRadius = 250.0f;

	// 효과 범위 검사에 포함할 Collision Object Type 목록이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Impact")
	TArray<TEnumAsByte<EObjectTypeQuery>> EffectObjectTypes;

	// 마지막 1회 범위 검사에서 대상별 Gameplay Event 전달 결과.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|GAS")
	TArray<FCPPotionEffectDispatchResult> LastDispatchResults;


	//여기서부턴 간단한 체크 함수 
	
	//충돌처리 가능한지 묻는 함수
	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool IsImpactProcessingEnabled() const;

	// 이 포션에서 첫 Impact가 이미 발생했는지 확인한다.
	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool HasPotionImpactTriggered() const;

	// 범위 안 Target ASC 중 하나 이상에서 효과 처리 Ability가 활성화됐는지 확인한다.
	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool HasActivatedAnyPotionEffectAbility() const;

private:
	// 첫 Impact를 잠그고 전달받은 Hit 문맥으로 효과 범위를 한 번 검사한다.
	bool TryCommitPotionImpact(const FHitResult& HitResult);

	// Impact 위치에서 Sphere Overlap을 한 번 실행하고 찾은 대상 ASC에 효과 Event를 전달한다.
	bool ResolvePotionEffectArea(const FHitResult& HitResult);

	// Potion.Effect 태그를 그대로 Gameplay Event로 사용해 Target ASC의 Ability를 활성화한다.
	bool TryDispatchPotionEffectToTarget(AActor* TargetActor, FGameplayTag EffectTag, const FHitResult& HitResult, UAbilitySystemComponent* SourceAbilitySystem, UAbilitySystemComponent* TargetAbilitySystem);

	// 실제 투척자 ASC를 우선 사용하고 더미 검증 Actor의 ASC를 보조 경로로 찾는다.
	UAbilitySystemComponent* ResolveSourceAbilitySystem() const;

	void AddDispatchResult(AActor* TargetActor, FGameplayTag EffectTag, ECPPotionEffectDispatchResult Result);
	
	// Context에 전달할 투척 원인 Pawn.
	UPROPERTY()
	TObjectPtr<APawn> ImpactInstigator = nullptr;

	// 포션 내부 저장 구조와 분리해 Impact가 전달받아 사용하는 현재 효과 태그.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	TArray<FGameplayTag> PotionEffectTags;

	// Impact 처리가 활성화된 뒤, 첫 Impact가 발생하거나 비활성화될 때까지 true.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bImpactProcessingEnabled = false;

	// 반복 Hit가 새로운 범위 검사를 만들지 않도록 첫 Impact를 잠근 상태.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bImpactTriggered = false;

	// 범위 안 Target ASC 중 하나 이상에서 효과 처리 Ability가 활성화된 상태.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bAnyEffectAbilityActivated = false;
};
