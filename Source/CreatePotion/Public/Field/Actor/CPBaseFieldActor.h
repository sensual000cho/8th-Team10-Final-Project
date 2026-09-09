#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "CPBaseFieldActor.generated.h"

class UGameplayAbility;
// 필드레벨에서 포션으로 상호작용 가능한 액터의 베이스
UCLASS()
class CREATEPOTION_API ACPBaseFieldActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	ACPBaseFieldActor();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	virtual void PotionReaction();
	virtual bool CanReactToTag(const FGameplayTag& EventTag) const;

protected:
	virtual void BeginPlay() override;
	
protected:
	// 반응할 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reaction")
	FGameplayTag ReactionEventTag;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> PotionReactionAbility;
};
