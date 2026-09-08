// CPEnemyCharacter.cpp


#include "Character/Enemy/CPEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

ACPEnemyCharacter::ACPEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	
}

void ACPEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACPEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


