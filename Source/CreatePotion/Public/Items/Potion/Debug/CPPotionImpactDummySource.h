#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "CPPotionImpactDummySource.generated.h"

class APawn;
class UAbilitySystemComponent;
class UCPForageableItemData;
class UCPPotionImpactComponent;
class UPrimitiveComponent;
struct FHitResult;

// 정식 Potion Actor의 효과 Tag 주입, 투척 활성화, 첫 Hit 전달 순서를 재현하는 테스트 Source.
UCLASS()
class CREATEPOTION_API ACPPotionImpactDummySource : public ACPAlchemyProp, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACPPotionImpactDummySource();

	// 더미용: Impact Component가 정상적인 ASC를 찾지 못했을 때 Source Actor의 대체 ASC를 얻으려고 호출한다.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Level Blueprint가 Spawn 직후 호출하며 정식 Potion Actor의 초기화와 투척 성공 시점을 더미용으로 구현한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Debug")
	bool PrepareDebugPotionImpact(APawn* InInstigator);

	// 더미용: Level Blueprint나 C++ 테스트가 물리 Hit 없이 지정한 위치에서 1회 범위 검사를 확인할 때 호출한다.
	UFUNCTION(BlueprintCallable, Category = "Potion|Debug")
	bool TryTriggerDebugPotionImpactAtLocation(FVector ImpactPoint, FVector ImpactNormal);

	// Unreal 물리 충돌이 호출하며 정식 Potion Actor의 Mesh Hit 전달에 대응한다.
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	// 정식 Potion Actor에도 같은 방식으로 소유할 공통 Impact Component.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	TObjectPtr<UCPPotionImpactComponent> PotionImpactComponent;

	// 플레이어 ASC가 아직 없는 테스트맵에서 Effect Context를 만들기 위한 더미 Source ASC.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	TObjectPtr<UAbilitySystemComponent> DebugSourceAbilitySystemComponent;

	// 기존 포션 데이터 초기화에 사용할 테스트용 ItemData
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion|Debug")
	TObjectPtr<UCPForageableItemData> TestItemData;

	// 더미 포션에 직접 주입할 테스트용 효과 Tag.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion|Debug")
	TArray<FGameplayTag> TestEffectTags;

	// 첫 Impact가 실제로 확정된 횟수이며 한 포션에서 최대 1이다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Debug")
	int32 ImpactTriggerCount = 0;

private:
	// NotifyHit이 전달한 첫 Hit를 공통 Impact Component에 넘긴다.
	bool TriggerDebugPotionImpact(const FHitResult& HitResult);

protected:
	// Unreal이 Spawn 뒤 호출하며 테스트용 Source ASC의 Owner와 Avatar를 초기화한다.
	virtual void BeginPlay() override;
};
