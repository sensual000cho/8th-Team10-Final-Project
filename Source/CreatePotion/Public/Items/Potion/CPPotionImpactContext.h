#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CPPotionImpactContext.generated.h"

class AActor;
class APawn;

USTRUCT(BlueprintType)
struct CREATEPOTION_API FCPPotionImpactContext
{
	GENERATED_BODY()

	// 포션을 실제로 던진 Actor
	UPROPERTY(BlueprintReadOnly, Category = "Potion|Impact")
	TObjectPtr<APawn> Instigator = nullptr;

	// Impact를 발생시킬 포션 Actor - 추후 Casting 다르게 하면 좋을 수도 있음
	UPROPERTY(BlueprintReadOnly, Category = "Potion|Impact")
	TObjectPtr<AActor> SourcePotion = nullptr;

	// 효과 적용을 요청받은 Actor
	UPROPERTY(BlueprintReadOnly, Category = "Potion|Impact")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Potion|Impact")
	FVector ImpactPoint = FVector::ZeroVector;
	
	// 맞은 표면에서 바깥쪽을 향하는 월드 공간 방향 벡터. 충돌 연출의 방향 정렬 용도임
	UPROPERTY(BlueprintReadOnly, Category = "Potion|Impact")
	FVector ImpactNormal = FVector::UpVector;
	
	//Potion 쪽의 CurrentEffects 순서 등등 유지해 전달하기
	UPROPERTY(BlueprintReadOnly, Category = "Potion|Impact")
	TArray<FGameplayTag> EffectTags;
};