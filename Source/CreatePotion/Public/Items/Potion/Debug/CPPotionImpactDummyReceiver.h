#pragma once

#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "CPPotionImpactDummyReceiver.generated.h"

class UBoxComponent;
class UAbilitySystemComponent;
class UGameplayAbility;
struct FGameplayEffectSpec;

// 정식 NPC나 Prop처럼 ASC와 Reaction GA를 미리 보유하고 포션 Gameplay Event를 받는 최소 GAS 대상 Actor.
UCLASS()
class CREATEPOTION_API ACPPotionImpactDummyReceiver : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Unreal이 Actor 기본 객체를 만들 때 호출하며 충돌체와 대상 ASC를 구성한다.
	ACPPotionImpactDummyReceiver();

	// Impact Component가 Target ASC를 찾을 때 IAbilitySystemInterface를 통해 호출한다.
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 포션과 실제 충돌할 수 있는 테스트용 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	TObjectPtr<UBoxComponent> CollisionBox;

	// GameplayEffect 수신과 Owned Tag 관리를 담당하는 더미 ASC.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// BeginPlay에서 더미 ASC에 넣는 면역 테스트 Tag. 정식 대상은 자체 ASC 초기화 정책을 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	FGameplayTagContainer InitialOwnedTags;

	// BeginPlay에서 GiveAbility할 테스트 GA. 정식 대상의 초기 Ability 부여와 같은 역할이다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	TArray<TSubclassOf<UGameplayAbility>> InitialAbilities;

	// 더미 ASC가 실제로 적용받은 GameplayEffect 횟수.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Debug")
	int32 ReceivedGameplayEffectCount = 0;

	// 가장 최근 GameplayEffect Spec이 가진 Asset Tag.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Debug")
	FGameplayTagContainer LastAppliedEffectTags;

protected:
	// Unreal이 Spawn 뒤 호출하며 ASC 초기화, 테스트 Tag, Reaction Ability 부여를 수행한다.
	virtual void BeginPlay() override;

private:
	// ASC Delegate가 GameplayEffect 적용 성공 뒤 호출하며 실제 Target GE 수신을 확인한다.
	void HandleGameplayEffectApplied(UAbilitySystemComponent* SourceAbilitySystem, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);
};
