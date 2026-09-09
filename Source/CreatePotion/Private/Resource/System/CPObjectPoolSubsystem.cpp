#include "Resource/System/CPObjectPoolSubsystem.h"
#include "GameCore/Interface/CPPoolable.h"

AActor* UCPObjectPoolSubsystem::AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& Transform)
{
	if (!ActorClass) return nullptr;
	
	FCPActorPool& Pool = Pools.FindOrAdd(ActorClass);
	
	AActor* Actor = nullptr;
	
	if (!Pool.AvailableActors.IsEmpty())
	{
		Actor = Pool.AvailableActors.Pop();
	}

	else
	{
		Actor = GetWorld()->SpawnActor<AActor>(ActorClass, Transform);
	}
	
	if (!Actor) return nullptr;
	
	Actor->SetActorTransform(Transform);
	
	if (Actor->Implements<UCPPoolable>())
	{
		ICPPoolable::Execute_OnAcquireFromPool(Actor);
	}
	
	return Actor;
}

void UCPObjectPoolSubsystem::ReleaseActor(AActor* Actor)
{
	if (!IsValid(Actor)) return;
	
	if (!Actor->Implements<UCPPoolable>())
	{
		Actor->Destroy();
		return;
	}
	
	ICPPoolable::Execute_OnReleaseToPool(Actor);
	
	FCPActorPool& Pool = Pools.FindOrAdd(Actor->GetClass());
	
	Pool.AvailableActors.Add(Actor);
}
