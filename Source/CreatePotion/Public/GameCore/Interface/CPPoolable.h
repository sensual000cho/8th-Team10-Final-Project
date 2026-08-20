#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPPoolable.generated.h"

UINTERFACE(MinimalAPI)
class UCPPoolable : public UInterface
{
	GENERATED_BODY()
};

// 오브젝트 풀링 인터페이스
class CREATEPOTION_API ICPPoolable
{
	GENERATED_BODY()

public:
	// 풀에서 꺼낼 때
	UFUNCTION(BlueprintNativeEvent)
	void OnAcquireFromPool();
	
	// 풀로 되돌릴 때
	UFUNCTION(BlueprintNativeEvent)
	void OnReleaseToPool();
};
