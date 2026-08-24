// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "CPTrajectoryPreviewComponent.generated.h"

/**
 * 
 */

class ACPThrowablePropBase;
class UCPCarryComponent;
class UGA_CPThrowProp;

struct FPredictProjectilePathResult;

UCLASS(ClassGroup = (Carry), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPTrajectoryPreviewComponent : public USplineComponent
{
	GENERATED_BODY()
	
public:
	UCPTrajectoryPreviewComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleHeldPropChanged(ACPThrowablePropBase* NewHeldProp);

	void ActivatePreview(ACPThrowablePropBase* InHeldProp);
	void DeactivatePreview();
	
	//충돌 크기 받아오기
	float ResolvePredictionRadius(const ACPThrowablePropBase* InHeldProp) const;
	
	bool CalculateTrajectory(FPredictProjectilePathResult& OutPathResult) const;
	void UpdatePreview();

	UPROPERTY()
	TObjectPtr<UCPCarryComponent> CarryComponent;

	UPROPERTY()
	TObjectPtr<ACPThrowablePropBase> HeldProp;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview")
	TSubclassOf<UGA_CPThrowProp> ThrowAbilityClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Appearance")
	TObjectPtr<UStaticMesh> PreviewSegmentMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Appearance")
	TObjectPtr<UMaterialInterface> PreviewMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Appearance", meta = (ClampMin = "1", UIMin = "1", UIMax = "32"))
	int32 MaxPreviewSegments = 16;

	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Appearance", meta = (ClampMin = "0.001"))
	float PreviewLineScale = 0.04f;
	
		// 정점 이후 가상 경로를 어디까지 계산할지 정하는 예측 범위.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path")
	float PostApexPredictionLength = 75.f;

	// 짧은 투척에서도 선 길이를 확보하기 위한 최소 가상 경로 계산 범위.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path")
	float MinPredictionLength = 150.f;

	// 선이 지나치게 길어지지 않도록 제한할 최대 가상 경로 계산 범위.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path")
	float MaxPredictionLength = 300.f;

	// Held Prop의 Collision 크기를 읽지 못했을 때 사용할 기본 반지름.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path")
	float DefaultPredictionRadius = 4.f;

	float CachedPredictionRadius = 4.f;
};
