#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/Potion/Interface/CPPotionEffectReceiver.h"
#include "CPPotionImpactDummyReceiver.generated.h"

class UBoxComponent;

UCLASS()
class CREATEPOTION_API ACPPotionImpactDummyReceiver : public AActor, public ICPPotionEffectReceiver
{
	GENERATED_BODY()

public:
	ACPPotionImpactDummyReceiver();

	virtual bool ReceivePotionImpact_Implementation(const FCPPotionImpactContext& Context) override;

	// 포션과 실제 충돌할 수 있는 테스트용 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Potion|Debug")
	TObjectPtr<UBoxComponent> CollisionBox;

	// true면 포션을 수용하고, false면 거부한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Potion|Debug")
	bool bAcceptPotionImpact = true;

	// ReceivePotionImpact가 실제로 호출된 횟수
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Debug")
	int32 ReceivedImpactCount = 0;

	// 가장 최근에 전달받은 포션 충돌 정보
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Potion|Debug")
	FCPPotionImpactContext LastImpactContext;
};
