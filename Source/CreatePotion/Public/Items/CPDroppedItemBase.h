#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameCore/Interface/CPPoolable.h"
#include "GameFramework/Actor.h"
#include "CPDroppedItemBase.generated.h"

class URotatingMovementComponent;
class UTimelineComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UCPForageableItemData;

// 바닥에 떨구는 아이템 베이스 액터
UCLASS()
class CREATEPOTION_API ACPDroppedItemBase : public AActor, public ICPInteractable, public ICPPoolable
{
	GENERATED_BODY()
	
public:	
	ACPDroppedItemBase();
	
	// 상호작용 인터페이스 관련
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	
	virtual FText GetInteractionPrompt_Implementation() override;
	
	virtual FName GetInteractionName_Implementation() override;
	
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	
	// 오브젝트 풀링 인터페이스 관련
	virtual void OnAcquireFromPool_Implementation() override;
	
	virtual void OnReleaseToPool_Implementation() override;
	
	// 아이템 데이터 초기화
	void Initialize(UCPForageableItemData* InItemData, int32 InAmount, UStaticMesh* InMesh);
	
	// 드랍 시 아이템 떨어지는 효과
	void StartDropMotion(const FVector& DropDirection);
	
protected:
	virtual void BeginPlay() override;
	
private:
	UFUNCTION()
	void OnDropMotionStopped(const FHitResult& ImpactResult);
	
	// 호버링	
	UFUNCTION()
	void UpdateHover(float Value);
	
	void StartHover();
	
private:
	// 현재 아이템 데이터가 재료밖에 없어서 재료 아이템 데이터로 함
	UPROPERTY(Transient)
	TObjectPtr<UCPForageableItemData> ItemData;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereCollision;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> VisualRoot;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Drop")
	TObjectPtr<UProjectileMovementComponent> DropMovement;
	
	UPROPERTY(VisibleAnywhere, Category = "Hover")
	TObjectPtr<UTimelineComponent> HoverTimeline;
	
	UPROPERTY(EditDefaultsOnly, Category = "Hover")
	TObjectPtr<UCurveFloat> HoverCurve;
	
	UPROPERTY(VisibleAnywhere, Category="Hover")
	TObjectPtr<URotatingMovementComponent> RotatingMovement;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DroppedScale = 0.8f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropHorizontalSpeed = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	float DropVerticalSpeed = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Hover")
	float HoverHeight = 20.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Hover")
	float HoverAmplitude = 10.f;
	
	int32 Amount = 1;
	
	FVector HoverBaseLocation;
};
