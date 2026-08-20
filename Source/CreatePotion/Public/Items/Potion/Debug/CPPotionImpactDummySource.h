#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "CPPotionImpactDummySource.generated.h"

class AActor;
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

	UFUNCTION(BlueprintCallable, Category = "Potion|Debug")
	bool TryApplyDebugPotionImpact(AActor* TargetActor);

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	TObjectPtr<UCPPotionImpactComponent> PotionImpactComponent;

	// 기존 포션 데이터 초기화에 사용할 테스트용 ItemData
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion|Debug")
	TObjectPtr<UCPForageableItemData> TestItemData;

	// 비어 있으면 TestItemData의 TagAxes를 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion|Debug")
	TArray<FGameplayTag> TestEffectTags;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Debug")
	int32 ImpactAttemptCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Debug")
	int32 SuccessfulImpactCount = 0;

private:
	bool ApplyDebugPotionImpact(AActor* TargetActor, const FHitResult& HitResult);
};
