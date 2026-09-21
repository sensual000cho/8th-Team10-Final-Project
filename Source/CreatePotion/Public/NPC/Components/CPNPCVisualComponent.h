#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CPNPCVisualComponent.generated.h"

class USkeletalMeshComponent;
class UCapsuleComponent;
class USkeletalMesh;
class UMaterialInterface;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FMeshOverrideEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag EffectTag;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY()
	TSubclassOf<UAnimInstance> OverrideAnimClass = nullptr;

	UPROPERTY()
	float CapsuleRadiusRatio = -1.0f;

	UPROPERTY()
	FVector MeshScale = FVector::OneVector;

	UPROPERTY()
	float OverrideCapsuleRadius = -1.0f;

	UPROPERTY()
	float OverrideCapsuleHalfHeight = -1.0f;

	UPROPERTY()
	FRotator OverrideRotationOffset = FRotator::ZeroRotator;

	FMeshOverrideEntry() {}
	FMeshOverrideEntry(FGameplayTag InTag, USkeletalMesh* InMesh, TSubclassOf<UAnimInstance> InAnimClass,
		float InRadiusRatio, const FVector& InMeshScale,
		float InOverrideCapsuleRadius, float InOverrideCapsuleHalfHeight,
		const FRotator& InRotationOffset)
		: EffectTag(InTag), Mesh(InMesh), OverrideAnimClass(InAnimClass),
		CapsuleRadiusRatio(InRadiusRatio), MeshScale(InMeshScale),
		OverrideCapsuleRadius(InOverrideCapsuleRadius), OverrideCapsuleHalfHeight(InOverrideCapsuleHalfHeight),
		OverrideRotationOffset(InRotationOffset) {
	}
};

USTRUCT(BlueprintType)
struct FMaterialOverrideEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag EffectTag;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> Material = nullptr;

	UPROPERTY()
	int32 Priority = 0; 

	FMaterialOverrideEntry() {}
	FMaterialOverrideEntry(FGameplayTag InTag, UMaterialInterface* InMat, int32 InPriority = 0)
		: EffectTag(InTag), Material(InMat), Priority(InPriority) {
	}
};

USTRUCT(BlueprintType)
struct FMaterialOverrideStack
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FMaterialOverrideEntry> Entries;
};

USTRUCT(BlueprintType)
struct FCompositeMaterialRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInterface> ResultMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 Priority = 0;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPNPCVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPNPCVisualComponent();

	void InitializeVisual(USkeletalMeshComponent* InMeshComp, UCapsuleComponent* InCapsuleComp,
		USkeletalMesh* DefaultMesh, TSubclassOf<UAnimInstance> InAnimClass,
		const FVector& BaseScale, float CapsuleRadiusRatio);

	// 메시 스택 관리
	void PushMeshOverride(FGameplayTag EffectTag, USkeletalMesh* InMesh, TSubclassOf<UAnimInstance> InAnimClass = nullptr, float InRadiusRatio = 0.3f,
		const FVector& InMeshScale = FVector::OneVector, float InOverrideCapsuleRadius = -1.0f,
		float InOverrideCapsuleHalfHeight = -1.0f, const FRotator& InRotationOffset = FRotator::ZeroRotator);
	void PopMeshOverride(FGameplayTag EffectTag);

	// 슬롯별 머티리얼 스택 관리
	void PushMaterialOverride(int32 SlotIndex, FGameplayTag EffectTag, UMaterialInterface* Material, int32 Priority = 0);
	void PopMaterialOverride(int32 SlotIndex, FGameplayTag EffectTag);

	// 복합 머티리얼 룰 등록 및 해제
	void RegisterCompositeRule(const FCompositeMaterialRule& Rule);
	void UnregisterCompositeRule(const FGameplayTagContainer& RequiredTags);

	// 스케일 및 캡슐 조정
	void SetScaleMultiplier(float Multiplier);
	FVector GetCurrentBaseScale() const;
	void FitCapsuleToCurrentMesh();

	// 래그돌 복원
	void RestoreMeshTransformAfterRagdoll();

	// 강제 비주얼 재적용
	void RefreshVisuals();

	// 거인화 배율
	UFUNCTION(BlueprintPure, Category = "Visual")
	float GetCurrentScaleMultiplier() const { return CurrentScaleMultiplier; }

	// 슬라임 전용 액세서리 부착 / 분리
	void AttachSlimeAccessory(UStaticMesh* InMesh, FName SocketName, const FTransform& RelativeTransform);
	void DetachSlimeAccessory();

private:
	void ApplyTopMesh();
	void ApplyTopMaterial(int32 SlotIndex);

private:
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY()
	TWeakObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> DefaultOriginalMesh;

	UPROPERTY()
	FRotator DefaultOriginalRotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector BaseMeshScale = FVector::OneVector;

	UPROPERTY()
	float StoredCapsuleRadiusRatio = 0.3f;

	UPROPERTY()
	float CurrentScaleMultiplier = 1.0f;

	UPROPERTY()
	TMap<int32, TObjectPtr<UMaterialInterface>> DefaultOriginalMaterials;

	UPROPERTY()
	TArray<FMeshOverrideEntry> MeshOverrideStack;

	UPROPERTY()
	TMap<int32, FMaterialOverrideStack> MaterialOverrideStackMap;

	UPROPERTY()
	TArray<FCompositeMaterialRule> CompositeRules;

	UPROPERTY()
	TSubclassOf<UAnimInstance> DefaultAnimClass;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SlimeAccessoryComp;

	UPROPERTY()
	TMap<int32, TObjectPtr<UMaterialInterface>> DefaultAccessoryMaterials;
};