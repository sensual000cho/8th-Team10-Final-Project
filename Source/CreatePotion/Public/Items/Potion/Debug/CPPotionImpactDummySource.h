#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "CPPotionImpactDummySource.generated.h"

class APawn;
class UCPForageableItemData;
class UCPPotionImpactComponent;
class UPrimitiveComponent;
struct FHitResult;

UCLASS()
class CREATEPOTION_API ACPPotionImpactDummySource : public ACPAlchemyProp
{
	GENERATED_BODY()

public:
	ACPPotionImpactDummySource();

	UFUNCTION(BlueprintCallable, Category = "Potion|Debug")
	bool PrepareDebugPotionImpact(APawn* InInstigator);

	// 물리 충돌 없이 지정한 위치에서 1회 Impact와 범위 검사를 확인한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Debug")
	bool TryTriggerDebugPotionImpactAtLocation(FVector ImpactPoint, FVector ImpactNormal);

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	TObjectPtr<UCPPotionImpactComponent> PotionImpactComponent;

	// 기존 포션 데이터 초기화에 사용할 테스트용 ItemData
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion|Debug")
	TObjectPtr<UCPForageableItemData> TestItemData;

	// 비어 있으면 TestItemData의 TagAxes를 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion|Debug")
	TArray<FGameplayTag> TestEffectTags;

	// 첫 Impact가 실제로 확정된 횟수이며 한 포션에서 최대 1이다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Debug")
	int32 ImpactTriggerCount = 0;

private:
	bool TriggerDebugPotionImpact(const FHitResult& HitResult);
};
