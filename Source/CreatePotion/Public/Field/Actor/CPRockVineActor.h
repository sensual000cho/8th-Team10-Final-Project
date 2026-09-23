#pragma once

#include "CoreMinimal.h"
#include "Field/Actor/CPBaseFieldActor.h"
#include "GameCore/Interface/CPPoolable.h"
#include "Resource/CPResourceType.h"
#include "CPRockVineActor.generated.h"

class UGameplayEffect;
class UNiagaraComponent;
// 뿌리 자라는 상태
UENUM()
enum class EVineScaleAnimState : uint8
{
	None,
	Growing,
	Shrinking
};

// 바위 틈새로 나무 뿌리가 튀어나와 있는 액터. 다음 태그에 반응.
// '성장' 태그가 있는 물약 : 나무 뿌리가 자라남.
// '뜨거움' 태그가 있는 물약 : 나무 뿌리에 불이 붙음. 일정 시간 이후 사라짐.
UCLASS()
class CREATEPOTION_API ACPRockVineActor : public ACPBaseFieldActor, public ICPPoolable
{
	GENERATED_BODY()
	
public:
	ACPRockVineActor();
	
	virtual void Tick(float DeltaSeconds) override;
	virtual bool PotionReaction(const FGameplayTag& EventTag, AActor* InstigatorActor = nullptr) override;
	
	void Initialize(const FCPResourceNodeKey& InNodeKey);
	
	// 오브젝트 풀링용
	virtual void OnAcquireFromPool_Implementation() override;
	virtual void OnReleaseToPool_Implementation() override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	// 컴포넌트 관리
	void CacheInitialVineState();
	
	// GAS State
	void BindGameplayTagEvents();
	void OnGiantStateChanged(const FGameplayTag Tag, int32 NewCount);
	void OnHeatStateChanged(const FGameplayTag Tag, int32 NewCount);
	void OnRespawningStateChanged(const FGameplayTag Tag, int32 NewCount);
	
	// 스케일
	void StartScaleTransition(float TargetMultiplier, float Duration);
	void SetOriginalScaleImmediately();
	
	// 화염 활성화
	void SetHeatFXActive(bool bActive);
	
	// 나무 뿌리 활성화
	void SetVinesEnabled(bool bEnabled);
	
	// 재생성
	void ApplyRespawnEffect();
	
	// 런타임 데이터 초기화
	void ResetRuntimeState();
	
	// 삭제 시 처리
	void HandleVineDestroyed();
	
protected:
	// 바위 메시 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> RockMesh;
	
	// 나무 뿌리 메시 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> VineMesh0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> VineMesh1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> VineMesh2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> VineMesh3;
	
	// 화염 나이아가라 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heat")
	TObjectPtr<UNiagaraComponent> HeatFXComponent0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heat")
	TObjectPtr<UNiagaraComponent> HeatFXComponent1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heat")
	TObjectPtr<UNiagaraComponent> HeatFXComponent2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heat")
	TObjectPtr<UNiagaraComponent> HeatFXComponent3;
	
	// 화염 나이아가라용 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heat")
	FName HeatFXComponentTag = TEXT("HeatFX");
	
	// 화염 재생성 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heat")
	TSubclassOf<UGameplayEffect> RespawnEffectClass;
	
	// 성장 관련 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Giant")
	float GiantScaleMultiplier = 2.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Giant")
	float GrowDuration = 0.8f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Giant")
	float ShrinkDuration = 0.8f;
	
private:
	// 성장 효과 복구를 위한 초기 정보
	TArray<FVector> OriginalScales;
	TArray<FVector> ScaleTransitionStart;
	TArray<FVector> ScaleTransitionTarget;
	TArray<ECollisionEnabled::Type> OriginalCollisionStates;
	TArray<bool> OriginalVisibilityStates;
	
	// 내부 순회용 컴포넌트 배열
	TArray<TObjectPtr<UStaticMeshComponent>> VineMeshes;
	TArray<TObjectPtr<UNiagaraComponent>> HeatFXComponents;
	
	// 성장 효과 도중 필요한 것들
	float ScaleTransitionElapsed = 0.f;
	float ScaleTransitionDuration = 0.f;
	bool bIsScaleTransitioning = false;
	
	// 채집물 감싸는 덩굴이라면,
	FCPResourceNodeKey NodeKey;
};
