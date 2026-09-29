#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "CPGA_PlayerSlime.generated.h"

class ACharacter;
class USkeletalMesh;
class USkeletalMeshComponent;
class UMeshComponent;
class UMaterialInterface;
class UAnimInstance;
class UCPHealthComponent;
class UCPInteractionComponent;

// 원래 이동과 애니메이션 구성은 유지하고 보이는 외형만 슬라임으로 바꾼다.
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
	// 임시 NPC 에셋은 BP에서 설정하고 나중에 교체한다.
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime")
	TObjectPtr<USkeletalMesh> SlimeMesh;
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime")
	TSubclassOf<UAnimInstance> SlimeAnimClass;
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime")
	TObjectPtr<UMaterialInterface> SlimeMaterial;

	// 원래 Character Mesh 기준 상대 변환. Giant 크기를 자동으로 따라간다.
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime")
	FTransform SlimeRelativeTransform;
	UPROPERTY(EditDefaultsOnly, Category = "Potion|Slime", meta = (ClampMin = "0.01", Units = "s"))
	float DurationSeconds = 30.0f;

private:
	void HandleDurationExpired();
	UFUNCTION()
	void HandlePlayerDeath(AActor* DamageCauser);
	// 정지 중 시선 회전은 무시하고 이동할 때만 외형 방향을 바꾼다.
	UFUNCTION()
	void UpdateSlimeFacing(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	// 효과가 숨긴 Mesh만 복구하여 원래 숨겨진 Mesh는 그대로 둔다.
	TArray<TWeakObjectPtr<UMeshComponent>> HiddenMeshes;
	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> SlimeVisual;
	TWeakObjectPtr<ACharacter> CachedCharacter;
	TWeakObjectPtr<UCPInteractionComponent> CachedInteraction;
	TWeakObjectPtr<UCPHealthComponent> CachedHealth;
	bool bOriginalInteractionBlocked = false;
	FTimerHandle DurationTimerHandle;
};
