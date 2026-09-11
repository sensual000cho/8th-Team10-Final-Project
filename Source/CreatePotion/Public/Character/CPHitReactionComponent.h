// CPHitReactionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPHitReactionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPHitReactionComponent();

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
