// Copyright Epic Games, Inc. All Rights Reserved.

#include "Settings/CPGameUserSettings.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameInstance/Subsystem/CPAudioSubsystem.h"

void UCPGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();
	ApplyAudioSettings();
}

void UCPGameUserSettings::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);
	ClampAudioVolumes();
}

void UCPGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	MasterVolume = 1.0f;
	SFXVolume = 1.0f;
	BGMVolume = 1.0f;
	UIVolume = 1.0f;
	AmbienceVolume = 1.0f;
}

void UCPGameUserSettings::ValidateSettings()
{
	Super::ValidateSettings();
	ClampAudioVolumes();
}

UCPGameUserSettings* UCPGameUserSettings::GetCPGameUserSettings()
{
	return Cast<UCPGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UCPGameUserSettings::ApplyAudioSettings() const
{
	if (!GEngine)
	{
		return;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* World = WorldContext.World();
		if (!IsValid(World) || !World->IsGameWorld())
		{
			continue;
		}

		UGameInstance* GameInstance = World->GetGameInstance();
		if (!IsValid(GameInstance))
		{
			continue;
		}

		if (UCPAudioSubsystem* AudioSubsystem = GameInstance->GetSubsystem<UCPAudioSubsystem>())
		{
			AudioSubsystem->ApplyVolumeSettings(MasterVolume, SFXVolume, BGMVolume, UIVolume, AmbienceVolume);
		}
	}
}

void UCPGameUserSettings::ApplyAndSaveAudioSettings()
{
	ClampAudioVolumes();
	ApplyAudioSettings();

	FSaveConfigContext SaveContext;
	SaveContext.Filename = GGameUserSettingsIni;
	SaveContext.bAllowCopyToDefaultObject = false;
	SaveContext.PropertyNames = {
		GET_MEMBER_NAME_CHECKED(UCPGameUserSettings, MasterVolume),
		GET_MEMBER_NAME_CHECKED(UCPGameUserSettings, SFXVolume),
		GET_MEMBER_NAME_CHECKED(UCPGameUserSettings, BGMVolume),
		GET_MEMBER_NAME_CHECKED(UCPGameUserSettings, UIVolume),
		GET_MEMBER_NAME_CHECKED(UCPGameUserSettings, AmbienceVolume)
	};
	SaveConfig(SaveContext);
}

void UCPGameUserSettings::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UCPGameUserSettings::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UCPGameUserSettings::SetBGMVolume(float Volume)
{
	BGMVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UCPGameUserSettings::SetUIVolume(float Volume)
{
	UIVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UCPGameUserSettings::SetAmbienceVolume(float Volume)
{
	AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

void UCPGameUserSettings::SetGraphicsLevel(int32 InLevel)
{
	GraphicsLevel = InLevel;
}

void UCPGameUserSettings::ClampAudioVolumes()
{
	MasterVolume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
	SFXVolume = FMath::Clamp(SFXVolume, 0.0f, 1.0f);
	BGMVolume = FMath::Clamp(BGMVolume, 0.0f, 1.0f);
	UIVolume = FMath::Clamp(UIVolume, 0.0f, 1.0f);
	AmbienceVolume = FMath::Clamp(AmbienceVolume, 0.0f, 1.0f);
}
