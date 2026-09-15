#include "NPC/GA/CPGA_Gust.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/Components/CPNPCMoveComponent.h"
#include "Data/NPC/CPNPCDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"

UCPGA_Gust::UCPGA_Gust()
{
	EffectTag = FGameplayTag::RequestGameplayTag(FName("State.Effect.Gust"));
	DurationWorldMinutes = 60; 
	Magnitude = 5.0f;         
}

void UCPGA_Gust::ApplyVisual(ACPBaseNPC* NPC, bool bActive, float InMagnitude)
{
	Super::ApplyVisual(NPC, bActive, InMagnitude);

	if (!IsValid(NPC))
	{
		return;
	}

	UCPNPCMoveComponent* MoveComp = NPC->FindComponentByClass<UCPNPCMoveComponent>();
	UCharacterMovementComponent* CharMoveComp = NPC->GetCharacterMovement();

	if (!MoveComp && !CharMoveComp)
	{
		return;
	}

	float BaseSpeed = 300.0f;
	if (const UCPNPCDataAsset* DA = NPC->GetNPCData())
	{
		BaseSpeed = DA->MoveSpeed;
	}
	else if (CharMoveComp)
	{
		BaseSpeed = CharMoveComp->MaxWalkSpeed;
	}

	if (bActive)
	{
		const float BuffedSpeed = BaseSpeed * InMagnitude;
		if (MoveComp)
		{
			MoveComp->ApplySpeed(BuffedSpeed);
		}
		else if (CharMoveComp)
		{
			CharMoveComp->MaxWalkSpeed = BuffedSpeed;
		}
	}
	else
	{
		if (MoveComp)
		{
			MoveComp->ApplySpeed(BaseSpeed);
		}
		else if (CharMoveComp)
		{
			CharMoveComp->MaxWalkSpeed = BaseSpeed;
		}
	}
}