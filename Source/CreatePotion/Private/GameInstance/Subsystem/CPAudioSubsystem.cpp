// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameInstance/Subsystem/CPAudioSubsystem.h"

#include "AudioDeviceHandle.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/CPAudioSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogCPAudio, Log, All);

void UCPAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UCPAudioSettings* AudioSettings = GetDefault<UCPAudioSettings>();
	VolumeControlMix = AudioSettings->VolumeControlMix.LoadSynchronous();
	MasterSoundClass = AudioSettings->MasterSoundClass.LoadSynchronous();
	SFXSoundClass = AudioSettings->SFXSoundClass.LoadSynchronous();
	BGMSoundClass = AudioSettings->BGMSoundClass.LoadSynchronous();
	UISoundClass = AudioSettings->UISoundClass.LoadSynchronous();
	AmbienceSoundClass = AudioSettings->AmbienceSoundClass.LoadSynchronous();
	VolumeFadeTime = FMath::Max(AudioSettings->VolumeFadeTime, 0.0f);

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
	ApplyVolumeMixForCurrentWorld();
}

void UCPAudioSubsystem::Deinitialize()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	ReleaseVolumeMix();

	Super::Deinitialize();
}

void UCPAudioSubsystem::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyVolumeMixForCurrentWorld();
}

void UCPAudioSubsystem::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyVolumeMixForCurrentWorld();
}

void UCPAudioSubsystem::SetBGMVolume(float Volume)
{
	BGMVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyVolumeMixForCurrentWorld();
}

void UCPAudioSubsystem::SetUIVolume(float Volume)
{
	UIVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyVolumeMixForCurrentWorld();
}

void UCPAudioSubsystem::SetAmbienceVolume(float Volume)
{
	AmbienceVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyVolumeMixForCurrentWorld();
}

void UCPAudioSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	ApplyVolumeMix(LoadedWorld);
}

void UCPAudioSubsystem::ApplyVolumeMixForCurrentWorld()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		ApplyVolumeMix(GameInstance->GetWorld());
	}
}

void UCPAudioSubsystem::ApplyVolumeMix(UWorld* World)
{
	if (!IsAudioWorld(World))
	{
		return;
	}

	const FAudioDeviceHandle AudioDevice = World->GetAudioDevice();
	if (!World->bAllowAudioPlayback || !AudioDevice.IsValid())
	{
		UE_LOG(LogCPAudio, Warning, TEXT("Volume SoundMix application deferred until audio playback is ready."));
		return;
	}

	if (!IsValid(VolumeControlMix) || !IsValid(MasterSoundClass) || !IsValid(SFXSoundClass) || !IsValid(BGMSoundClass) || !IsValid(UISoundClass) || !IsValid(AmbienceSoundClass))
	{
		UE_LOG(LogCPAudio, Warning, TEXT("Volume SoundMix or required SoundClass asset is missing."));
		return;
	}

	const uint32 AudioDeviceId = AudioDevice.GetDeviceID();
	if (bVolumeMixPushed && VolumeMixAudioDeviceId != AudioDeviceId)
	{
		bVolumeMixPushed = false;
		VolumeMixAudioDeviceId = MAX_uint32;
	}

	if (!bVolumeMixPushed)
	{
		UGameplayStatics::PushSoundMixModifier(World, VolumeControlMix);
		bVolumeMixPushed = true;
		VolumeMixAudioDeviceId = AudioDeviceId;
		UE_LOG(LogCPAudio, Log, TEXT("Volume SoundMix pushed to audio device %u."), VolumeMixAudioDeviceId);
	}

	UGameplayStatics::SetSoundMixClassOverride(World, VolumeControlMix, MasterSoundClass, MasterVolume, 1.0f, VolumeFadeTime, true);
	UGameplayStatics::SetSoundMixClassOverride(World, VolumeControlMix, SFXSoundClass, SFXVolume, 1.0f, VolumeFadeTime, true);
	UGameplayStatics::SetSoundMixClassOverride(World, VolumeControlMix, BGMSoundClass, BGMVolume, 1.0f, VolumeFadeTime, true);
	UGameplayStatics::SetSoundMixClassOverride(World, VolumeControlMix, UISoundClass, UIVolume, 1.0f, VolumeFadeTime, true);
	UGameplayStatics::SetSoundMixClassOverride(World, VolumeControlMix, AmbienceSoundClass, AmbienceVolume, 1.0f, VolumeFadeTime, true);
}

void UCPAudioSubsystem::ReleaseVolumeMix()
{
	if (!bVolumeMixPushed || !IsValid(VolumeControlMix))
	{
		bVolumeMixPushed = false;
		VolumeMixAudioDeviceId = MAX_uint32;
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWorld* World = GameInstance->GetWorld(); IsAudioWorld(World))
		{
			const FAudioDeviceHandle AudioDevice = World->GetAudioDevice();
			if (World->bAllowAudioPlayback && AudioDevice.IsValid() && AudioDevice.GetDeviceID() == VolumeMixAudioDeviceId)
			{
				UGameplayStatics::PopSoundMixModifier(World, VolumeControlMix);
			}
		}
	}

	bVolumeMixPushed = false;
	VolumeMixAudioDeviceId = MAX_uint32;
}

bool UCPAudioSubsystem::IsAudioWorld(const UWorld* World) const
{
	return IsValid(World) && World->IsGameWorld() && World->GetGameInstance() == GetGameInstance();
}
