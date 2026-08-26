#include "Environment/Time/CPTimeOfDayController.h"
#include "Engine/DirectionalLight.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"

ACPTimeOfDayController::ACPTimeOfDayController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACPTimeOfDayController::BeginPlay()
{
	Super::BeginPlay();
	
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>();
	if (!TimeSubsystem) return;
	
	TargetGameMinutes = TimeSubsystem->GetTotalWorldMinutes();
	VisualGameMinutes = TargetGameMinutes;
	
	UpdateSunRotation(VisualGameMinutes);
	
	TimeSubsystem->OnTimeChanged.AddDynamic(this, &ACPTimeOfDayController::HandleTimeChanged);
	
	GetWorldTimerManager().SetTimer(
		VisualUpdateTimerHandle,
		this,
		&ACPTimeOfDayController::UpdateVisualTime,
		0.1f,
		true);
}

void ACPTimeOfDayController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TimeSubsystem)
	{
		TimeSubsystem->OnTimeChanged.RemoveAll(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ACPTimeOfDayController::HandleTimeChanged(int64 NewMinutes)
{
	TargetGameMinutes = NewMinutes;
}

void ACPTimeOfDayController::UpdateSunRotation(double GameMinutes)
{
	if (!SunLight) return;
	
	constexpr double MinutesPerDay = 144.0;
	
	const double MinutesOfDay = FMath::Fmod(GameMinutes, MinutesPerDay);
	const double DayAlpha = MinutesOfDay / MinutesPerDay;
	const float SunPitch = SunPitchOffset + static_cast<float>(DayAlpha * 360.0);
	const float MoonPitch = SunPitch + 180.f;
	
	if (SunLight)
	{
		SunLight->SetActorRotation(FRotator(SunPitch, SunYaw, 0.f));
	}
	
	if (MoonLight)
	{
		MoonLight->SetActorRotation(FRotator(MoonPitch, SunYaw, 0.f));
	}
}

void ACPTimeOfDayController::UpdateVisualTime()
{
	VisualGameMinutes = FMath::FInterpConstantTo(
		VisualGameMinutes,
		TargetGameMinutes,
		0.1f,
		1.f);
	
	UpdateSunRotation(VisualGameMinutes);
}


