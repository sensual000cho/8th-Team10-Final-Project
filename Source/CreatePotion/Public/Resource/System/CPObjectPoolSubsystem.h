#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CPObjectPoolSubsystem.generated.h"

// 개별 풀 구조체
USTRUCT()
struct FCPActorPool
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AvailableActors;
};

// 오브젝트 풀링 WorldSubsystem
UCLASS()
class CREATEPOTION_API UCPObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& Transform);
	
	void ReleaseActor(AActor* Actor);
	
private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FCPActorPool> Pools;
};
