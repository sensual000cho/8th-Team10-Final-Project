#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPTimeOfDayController.generated.h"

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
	// Sun Light
	UPROPERTY(EditAnywhere, Category = "Time")
	TObjectPtr<ADirectionalLight> SunLight;
	
	// Moon Light
	UPROPERTY(EditAnywhere, Category = "Time")
	TObjectPtr<ADirectionalLight> MoonLight;
	
	// 하늘 액터
	UPROPERTY(EditAnywhere, Category = "Time")
	TObjectPtr<AActor> SkySphere;
	
	// 태양 궤도 시작 각도
	UPROPERTY(EditAnywhere, Category = "Time")
	float SunPitchOffset = -90.f;
	
	// 방향 조절용
	UPROPERTY(EditAnywhere, Category = "Time")
	float SunYaw = 0.f;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPTimeSubsystem> TimeSubsystem;
	
	// 화면에 표시된 현재 시간
	double VisualGameMinutes = 0.0;
	
	// 실제 게임 시간(목표 시간)
	double TargetGameMinutes = 0.0;
	
	FTimerHandle VisualUpdateTimerHandle;
};
