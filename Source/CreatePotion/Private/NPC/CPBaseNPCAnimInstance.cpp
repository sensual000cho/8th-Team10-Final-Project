#include "NPC/CPBaseNPCAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NPC/Components/CPNPCVisualComponent.h"
#include "NPC/CPBaseNPC.h"

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
		if (!OwnerPawn) return;
	}

	const float RawSpeed = OwnerPawn->GetVelocity().Size2D();
	bShouldMove = (RawSpeed > 3.0f);

	float Multiplier = 1.0f;
	if (ACPBaseNPC* BaseNPC = Cast<ACPBaseNPC>(OwnerPawn))
	{
		if (UCPNPCVisualComponent* VisualComp = BaseNPC->GetVisualComponent())
		{
			Multiplier = VisualComp->GetCurrentScaleMultiplier();
		}
	}

	if (Multiplier >= 2.0f)
	{
		const float SafeDivisor = FMath::Max(0.1f, GiantSpeedDivisor);
		GroundSpeed = RawSpeed / SafeDivisor;
	}
	else
	{
		GroundSpeed = RawSpeed;
	}
}