#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "CPPotionImpactComponent.generated.h"

class AActor;
class APawn;
struct FCPPotionImpactContext;


// 포션의 첫 충돌 지점에서 효과 범위를 한 번 검사하고 Receiver에게 Context를 전달한다.
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
	bool TryTriggerPotionImpact(const FHitResult& HitResult);

	// 물리 Hit 없이 지정한 위치에서 동일한 1회 효과 범위 검사를 실행한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	bool TryTriggerPotionImpactAtLocation(FVector ImpactPoint, FVector ImpactNormal);

	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool IsImpactProcessingEnabled() const;

	// 이 포션에서 첫 Impact가 이미 발생했는지 확인한다.
	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool HasPotionImpactTriggered() const;

	// Impact 위치의 효과 범위 검사가 끝났는지 확인한다.
	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool IsEffectAreaResolved() const;

	// 범위 안 Receiver 중 하나 이상이 효과 적용에 성공했는지 확인한다.
	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool HasAppliedAnyPotionEffect() const;

	// 첫 충돌 시 Receiver를 찾을 구형 범위의 반지름이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Impact", meta = (ClampMin = "0.0", Units = "cm"))
	float EffectRadius = 250.0f;

	// 효과 범위 검사에 포함할 Collision Object Type 목록이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Impact")
	TArray<TEnumAsByte<EObjectTypeQuery>> EffectObjectTypes;

private:
	// Impact 위치에서 Sphere Overlap을 한 번 실행하고 찾은 Receiver들을 호출한다.
	bool ResolvePotionEffectArea(const FVector& ImpactPoint, const FVector& ImpactNormal);

	// 대상별 포션 정보를 Context 구조체로 패키징한다.
	bool TryBuildImpactContext(AActor* TargetActor, const FVector& ImpactPoint, const FVector& ImpactNormal, FCPPotionImpactContext& OutContext) const;
	
	// Context에 전달할 투척 원인 Pawn.
	UPROPERTY()
	TObjectPtr<APawn> ImpactInstigator = nullptr;

	// Impact 처리가 활성화된 뒤, 첫 Impact가 발생하거나 비활성화될 때까지 true.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bImpactProcessingEnabled = false;

	// 반복 Hit가 새로운 범위 검사를 만들지 않도록 첫 Impact를 잠근 상태.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bImpactTriggered = false;

	// 첫 Impact 위치의 Sphere Overlap 검사가 끝난 상태.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bEffectAreaResolved = false;

	// 범위 안 Receiver 중 하나 이상이 실제 효과 적용에 성공한 상태.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bAnyEffectApplied = false;
};
