// CPEnemyCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPEnemyCharacter.generated.h"

UCLASS()
class CREATEPOTION_API ACPEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACPEnemyCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
