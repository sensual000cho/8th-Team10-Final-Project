#include "Environment/Time/CPTimeOfDayController.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"

ACPTimeOfDayController::ACPTimeOfDayController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(Root);
	
	MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
	MoonLight->SetupAttachment(Root);
	
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(Root);
	
	HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
	HeightFog->SetupAttachment(Root);
	
	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloud"));
	VolumetricCloud->SetupAttachment(Root);
	
	SkySphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkySphere"));
	SkySphere->SetupAttachment(Root);
	
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(Root);
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
		SunLight->SetWorldRotation(FRotator(SunPitch, SunYaw, 0.f));
	}
	
	if (MoonLight)
	{
		MoonLight->SetWorldRotation(FRotator(MoonPitch, SunYaw, 0.f));
	}
	
	if (SkySphere)
	{
		UMaterialInstanceDynamic* Material = SkySphere->CreateDynamicMaterialInstance(0);
		if (Material)
		{
			float Intensity = FMath::GetMappedRangeValueClamped(
			FVector2D(-0.1f, 0.1f),
			FVector2D(0.f, 1.f),
			FMath::Sin(FMath::DegreesToRadians(SunPitch)));
				
			Material->SetScalarParameterValue(TEXT("SkyIntensity"), Intensity);
		}
	}
	
	if (SkyLight && SkyLightIntensityCurve)
	{
		const float SkyLightIntensity = SkyLightIntensityCurve->GetFloatValue(DayAlpha);
		
		SkyLight->SetIntensity(SkyLightIntensity);
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


