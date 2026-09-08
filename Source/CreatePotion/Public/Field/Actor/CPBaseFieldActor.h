#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "CPBaseFieldActor.generated.h"

// 필드레벨에서 포션으로 상호작용 가능한 액터의 베이스
UCLASS()
class CREATEPOTION_API ACPBaseFieldActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	ACPBaseFieldActor();

protected:
	virtual void BeginPlay() override;
};
