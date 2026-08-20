#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "CPPotionImpactComponent.generated.h"

class AActor;
class APawn;
struct FCPPotionImpactContext;


//포션 투척 - Impact 후, 효과 적용을 한 번 시도하고, 성공하면 소비 했다는 형식으로 return 하는 역할
UCLASS(ClassGroup=(Potion), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPPotionImpactComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPPotionImpactComponent();

	// 포션이 실제로 투척된 뒤 Impact 처리를 허용한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	bool EnableImpactProcessing(APawn* InInstigator);

	// Impact 처리를 중단한다. 이미 소비된 상태는 되돌리지 않는다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	void DisableImpactProcessing();

	// 포션 Actor가 Hit 했을 때 호출하는 함수
	UFUNCTION(BlueprintCallable, Category = "Potion|Impact")
	bool TryApplyPotionImpact(AActor* TargetActor, const FHitResult& HitResult);

	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool IsImpactProcessingEnabled() const;

	//이미 포션 Impact가 성공 했는지 체크
	UFUNCTION(BlueprintPure, Category = "Potion|Impact")
	bool IsPotionConsumed() const;

private:
	//포션 정보 구조체로 패키징
	bool TryBuildImpactContext(AActor* TargetActor, const FHitResult& HitResult, FCPPotionImpactContext& OutContext) const;
	
	// Context에 전달할 투척 원인 Pawn.
	UPROPERTY()
	TObjectPtr<APawn> ImpactInstigator = nullptr;

	// Impact 처리가 활성화된 뒤, 효과 적용에 성공하거나 비활성화될 때까지 true
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bImpactProcessingEnabled = false;

	// 성공한 포션을 다시 적용하지 않기 위한 소비 상태.
	UPROPERTY(VisibleInstanceOnly, Category = "Potion|Impact")
	bool bConsumed = false;

	// 같은 포션 Component가 Receiver 처리 중 다시 호출되는 것을 막음
	bool bApplyingImpact = false;
	

};
