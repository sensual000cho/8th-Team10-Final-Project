#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPTimeOfDayController.generated.h"

class UPostProcessComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;
class UVolumetricCloudComponent;
class ADirectionalLight;
class UCPTimeSubsystem;

// 시간에 따른 환경 변화 연출용 액터
UCLASS()
class CREATEPOTION_API ACPTimeOfDayController : public AActor
{
	GENERATED_BODY()
	
public:	
	ACPTimeOfDayController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	UFUNCTION()
	void HandleTimeChanged(int64 NewMinutes);
	
	void UpdateSunRotation(double GameMinutes);
	
	void UpdateVisualTime();
	
private:
	// Root Component
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;
	
	// Sun Light
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<UDirectionalLightComponent> SunLight;
	
	// Moon Light
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<UDirectionalLightComponent> MoonLight;
	
	// Sky Light
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<USkyLightComponent> SkyLight;
	
	// Sky Atmosphere
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;
	
	// Height Fog
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<UExponentialHeightFogComponent> HeightFog;
	
	// Volumetric Cloud
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloud;
	
	// 하늘 메시
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<UStaticMeshComponent> SkySphere;
	
	// 포스트 프로세스
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<UPostProcessComponent> PostProcess;
	
	// Sky Light Intensity Curve
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	TObjectPtr<UCurveFloat> SkyLightIntensityCurve;
	
	// 태양 궤도 시작 각도
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	float SunPitchOffset = -90.f;
	
	// 방향 조절용
	UPROPERTY(EditAnywhere, Category = "Time Of Day")
	float SunYaw = 0.f;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPTimeSubsystem> TimeSubsystem;
	
	// 화면에 표시된 현재 시간
	double VisualGameMinutes = 0.0;
	
	// 실제 게임 시간(목표 시간)
	double TargetGameMinutes = 0.0;
	
	FTimerHandle VisualUpdateTimerHandle;
};
