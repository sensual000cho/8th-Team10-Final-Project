// HitReactionComponent.cpp


#include "Character/HitReactionComponent.h"

#include "Character/Enemy/CPEnemyCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UHitReactionComponent::UHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHitReactionComponent::HitReaction(float AppliedDamage, AActor* DamageCauser)
{
	ACPEnemyCharacter* Enemy = Cast<ACPEnemyCharacter>(DamageCauser);
		
	if (!Enemy || !Player)
	{
		return;
	}
	
	UCharacterMovementComponent* CMC =  Player->GetCharacterMovement();
	if (!CMC)
	{
		return;
	}
	
	FVector Direction = (Player->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal2D();
	CMC->AddImpulse(Direction * KnockbackStrength, true);
}


void UHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Player = Cast<ACharacter>(GetOwner());
}


