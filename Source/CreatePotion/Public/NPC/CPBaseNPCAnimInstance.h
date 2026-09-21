#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CPBaseNPCAnimInstance.generated.h"

class UCharacterMovementComponent;

UCLASS()
class CREATEPOTION_API UCPBaseNPCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Scale")
	float GiantSpeedDivisor = 3.0f;

private:
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent;
};