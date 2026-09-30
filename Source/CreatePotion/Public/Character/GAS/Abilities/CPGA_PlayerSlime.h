#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PlayerSlime.generated.h"

class USoundBase;

class ACharacter;
class USkeletalMesh;
class USkeletalMeshComponent;
class UMeshComponent;
class UMaterialInterface;
class UAnimInstance;
class UCPHealthComponent;
class UCPInteractionComponent;

/** Player의 이동 구성을 유지하며 슬라임 외형과 상호작용 제한을 적용한다. */
UCLASS(Blueprintable)
class CREATEPOTION_API UCPGA_PlayerSlime : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCPGA_PlayerSlime();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 효과 적용 성공 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> StartSFX;

	// 효과 종료 시 재생할 단발음. 미지정 시 생략
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Potion")
	TObjectPtr<USoundBase> EndSFX;

	// 변신 중 표시할 슬라임 메시
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime")
	TObjectPtr<USkeletalMesh> SlimeMesh;

	// 슬라임 메시의 애니메이션 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime")
	TSubclassOf<UAnimInstance> SlimeAnimClass;

	// 미지정 시 슬라임 메시의 기본 머티리얼 사용
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime")
	TObjectPtr<UMaterialInterface> SlimeMaterial;

	// Character Mesh 기준 상대 변환. 부모 메시의 크기 변화 반영
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime")
	FTransform SlimeRelativeTransform;

	// Slime 유지 시간. 활성 중 재적용 시 갱신하지 않음
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime", meta = (ClampMin = "0.01", Units = "s"))
	float DurationSeconds = 30.0f;

private:
	// 시작에 성공한 실행만 종료음을 한 번 재생
	bool bEffectSFXStarted = false;

	void HandleDurationExpired();
	UFUNCTION()
	void HandlePlayerDeath(AActor* DamageCauser);
	// 수평 이동 방향으로 외형 회전, 정지 중 방향 유지
	UFUNCTION()
	void UpdateSlimeFacing(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	// 종료 시 복구할 메시 목록. 변신 전부터 숨겨진 메시는 제외
	TArray<TWeakObjectPtr<UMeshComponent>> HiddenMeshes;
	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> SlimeVisual;
	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<UCPInteractionComponent> CachedInteraction;
	TWeakObjectPtr<UCPHealthComponent> CachedHealth;
	bool bOriginalInteractionBlocked = false;
	FTimerHandle DurationTimerHandle;
};
