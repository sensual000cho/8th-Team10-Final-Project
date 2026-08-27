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
class UMaterialInterface;
class USplineMeshComponent;
class UStaticMesh;

struct FPredictProjectilePathResult;

UCLASS(ClassGroup = (Carry), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPTrajectoryPreviewComponent : public USplineComponent
{
	GENERATED_BODY()
	
public:
	UCPTrajectoryPreviewComponent();
	
	void ActivatePreview(ACPThrowablePropBase* InHeldProp);
	void DeactivatePreview();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleHeldPropChanged(ACPThrowablePropBase* NewHeldProp);
	
	//충돌 크기 받아오기
	float ResolvePredictionRadius(const ACPThrowablePropBase* InHeldProp) const;
	
	bool CalculateTrajectory(FPredictProjectilePathResult& OutPathResult) const;
	void UpdatePreview();
	
	void CreatePreviewSegments();
	void UpdatePreviewSegments();
	void HidePreviewSegments();

	UPROPERTY()
	TObjectPtr<UCPCarryComponent> CarryComponent;

	UPROPERTY()
	TObjectPtr<ACPThrowablePropBase> HeldProp;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview")
	TSubclassOf<UGA_CPThrowProp> ThrowAbilityClass;
	
	// Spline 경로를 따라 휘어서 표시할 선 조각용 Static Mesh.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Appearance")
	TObjectPtr<UStaticMesh> PreviewSegmentMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Appearance")
	TObjectPtr<UMaterialInterface> PreviewMaterial;

	// BeginPlay에서 한 번 생성한 뒤 재사용할 선 조각의 최대 개수.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Appearance", meta = (ClampMin = "1", UIMin = "1", UIMax = "32"))
	int32 MaxPreviewSegments = 16;

	// Static Mesh 원본 크기를 기준으로 적용할 선 단면의 배율.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Appearance", meta = (ClampMin = "0.001"))
	float PreviewLineScale = 0.02f;
	
	// 정점 이후 가상 경로를 어디까지 계산할지 정하는 예측 범위.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path")
	float PostApexPredictionLength = 150.f;

	// 짧은 투척에서도 선 길이를 확보하기 위한 최소 가상 경로 계산 범위.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path")
	float MinPredictionLength = 300.f;

	// 선이 지나치게 길어지지 않도록 제한할 최대 가상 경로 계산 범위.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path")
	float MaxPredictionLength = 600.f;

	// Held Prop의 Collision 크기를 읽지 못했을 때 사용할 기본 반지름.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path")
	float DefaultPredictionRadius = 4.f;
	
	// Held Prop 위치부터 이 거리만큼은 궤적을 표시하지 않음.
	UPROPERTY(EditDefaultsOnly, Category = "Trajectory Preview|Path", meta = (ClampMin = "0.0", Units = "cm"))
	float PreviewStartDistance = 50.f;

	float CachedPredictionRadius = 4.f;
	
	//Spline 선 조각들 모음집
	UPROPERTY(Transient)
	TArray<TObjectPtr<USplineMeshComponent>> PreviewSegments;
};
