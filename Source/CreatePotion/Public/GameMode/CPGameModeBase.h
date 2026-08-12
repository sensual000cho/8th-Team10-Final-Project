// CPGameModeBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CPGameModeBase.generated.h"

UCLASS()
class CREATEPOTION_API ACPGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ACPGameModeBase();
	
protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	
private:
	APlayerStart* FindPlayerStartByTag(FName SpawnPointId) const;
};
