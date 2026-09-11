// HitReactionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitReactionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHitReactionComponent();

	UFUNCTION(BlueprintCallable, Category = "Damaged")
	void HitReaction(float AppliedDamage, AActor* DamageCauser);
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<ACharacter> Player;
	
	UPROPERTY(EditDefaultsOnly, Category = "Damaged", meta = (AllowPrivateAccess))
	FVector KnockbackStrength = FVector(300.f, 300.f, 0.f);
};
