#include "NPC/CPBaseNPCAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCPBaseNPCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerPawn = TryGetPawnOwner();
	if (OwnerPawn)
	{
		if (ACharacter* Char = Cast<ACharacter>(OwnerPawn))
		{
			MovementComponent = Char->GetCharacterMovement();
		}
	}
}

void UCPBaseNPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerPawn)
	{
		OwnerPawn = TryGetPawnOwner();
		if (OwnerPawn)
		{
			MovementComponent = Cast<ACharacter>(OwnerPawn) ? Cast<ACharacter>(OwnerPawn)->GetCharacterMovement() : nullptr;
		}
		if (!OwnerPawn) return;
	}
	GroundSpeed = OwnerPawn->GetVelocity().Size2D();
	bShouldMove = (GroundSpeed > 3.0f);
}