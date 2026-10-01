#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NPC/GA/CPGA_ImpulseReactionBase.h"
#include "CPGA_Explosion.generated.h"

class UAbilitySystemComponent;
struct FBodyInstance;

/**
 * 포션 폭발 리액션.
 * 1) 레그돌 상태로 태풍(바람장)에 휘말려 폭발 중심(피격 지점 + Z 오프셋)으로 나선형으로 빨려 들어옴
 * 2) 궤도 반지름(OrbitRadius)에서 잠시 회전 유지
 * 3) 중심에서 바깥(+위)으로 폭발하며 날아감 -> 자연 정착 후 종료
 *
 * 코어 바디(질량이 가장 큰 바디)만 바람장(수렴 + 중심 둘레 회전) 속도로 구동하고,
 * 팔다리는 조인트에 끌려오며 자유롭게 휘날림. 속도 변화량은 가속도 상한으로 제한해 관절 늘어남을 방지.
 */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_Explosion : public UCPGA_ImpulseReactionBase
{
	GENERATED_BODY()

public:
	UCPGA_Explosion();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual void CleanupNPCReaction(const FGameplayAbilityActorInfo* ActorInfo) override;
	virtual float GetMeshBounceMinSpeed() const override { return BounceMinSpeed; }
	virtual float GetMeshBounceSpinMultiplier() const override { return BounceSpin; }

private:
	void TickPull();
	void Explode(USkeletalMeshComponent* Mesh);
	void FinishExplosionForce();
	FVector GetCorePosition(USkeletalMeshComponent* Mesh) const;
	void GetBodyState(USkeletalMeshComponent* Mesh, FVector& OutPos, FVector& OutVel, float& OutMass) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float PullDelay = 0.5f;

	// 피격 지점에서 Z로 올릴 높이
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Pull")
	float CenterZOffset = 150.0f;

	// 궤도 반지름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float OrbitRadius = 120.0f;

	// NPC마다 궤도가 겹치지 않도록 주는 반지름 랜덤 비율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Pull", meta = (ClampMin = "0", ClampMax = "0.5"))
	float OrbitRadiusJitter = 0.2f;

	// 회전 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Pull")
	float OrbitSpeedDegPerSec = 300.0f;

	// 끌려 들어오기 시작해서 터질 때까지의 전체 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Pull", meta = (ClampMin = "0.05"))
	float PullDuration = 2.0f;

	// 목표 반지름/높이로 수렴하는 속도 계수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float PullStiffness = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Pull", meta = (ClampMin = "0"))
	float MaxPullSpeed = 2000.0f;

	// 코어가 바람 속도로 끌려가는 강도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Wind", meta = (ClampMin = "0"))
	float WindFollowGain = 8.0f;

	// 몸 전체를 끌어오는 힘 중 코어 바디에 몰아주는 비율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Wind", meta = (ClampMin = "0", ClampMax = "1"))
	float CoreDriveRatio = 0.6f;

	// 몸 전체 질량 중심에 가해지는 최대 가속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Wind", meta = (ClampMin = "0"))
	float MaxPullAccel = 6000.0f;

	// 코어에 랜덤 회전 충격을 주는 세기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Wind", meta = (ClampMin = "0"))
	float TurbulenceSpinDegPerSec = 200.0f;

	// 랜덤 회전 충격 주기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Wind", meta = (ClampMin = "0.02"))
	float TurbulenceInterval = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Blast")
	float ExplosionSpeed = 1800.0f;

	// 폭발 시 루트 외 나머지 본이 받는 속도 비율 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Blast", meta = (ClampMin = "0", ClampMax = "1"))
	float ExplosionLimbSpeedRatio = 0.6f;

	// 바깥 방향에 더하는 위쪽 성분
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Blast")
	float ExplosionUpwardBias = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Blast")
	float ExplosionSpin = 300.0f;

	// 폭발 직후 벽/바닥 바운스 반응을 유지하는 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Blast")
	float ExplosionBounceDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Blast")
	float BounceMinSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Blast")
	float BounceSpin = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Cue")
	FGameplayTag GatherCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Cue")
	FGameplayTag ExplodeCueTag;

private:
	FVector Center = FVector::ZeroVector;
	float StartRadius = 0.0f;
	float ActualOrbitRadius = 0.0f;
	float StartZ = 0.0f;
	float PullStartTime = 0.0f;
	float LastTickTime = 0.0f;
	float TurbulenceTimer = 0.0f;
	FBodyInstance* CoreBody = nullptr;

	FTimerHandle PullTimerHandle;
	FTimerHandle ExplosionTimerHandle;

};