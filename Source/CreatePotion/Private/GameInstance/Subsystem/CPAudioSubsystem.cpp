// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameInstance/Subsystem/CPAudioSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogCPAudio, Log, All);

namespace CPAudio
{
	const TCHAR* VolumeControlMixPath = TEXT("/Game/CreatePotion/Audio/SoundMixes/MIX_VolumeControl.MIX_VolumeControl");
	const TCHAR* MasterSoundClassPath = TEXT("/Game/CreatePotion/Audio/SoundClasses/SC_Master.SC_Master");
	const TCHAR* SFXSoundClassPath = TEXT("/Game/CreatePotion/Audio/SoundClasses/SC_SFX.SC_SFX");
	const TCHAR* BGMSoundClassPath = TEXT("/Game/CreatePotion/Audio/SoundClasses/SC_BGM.SC_BGM");
	constexpr float VolumeFadeTime = 0.05f;
}

UCPAudioSubsystem::UCPAudioSubsystem()
{
	static ConstructorHelpers::FObjectFinder<USoundMix> VolumeControlMixFinder(CPAudio::VolumeControlMixPath);
	VolumeControlMix = VolumeControlMixFinder.Object;

	static ConstructorHelpers::FObjectFinder<USoundClass> MasterSoundClassFinder(CPAudio::MasterSoundClassPath);
	MasterSoundClass = MasterSoundClassFinder.Object;

	static ConstructorHelpers::FObjectFinder<USoundClass> SFXSoundClassFinder(CPAudio::SFXSoundClassPath);
	SFXSoundClass = SFXSoundClassFinder.Object;

	static ConstructorHelpers::FObjectFinder<USoundClass> BGMSoundClassFinder(CPAudio::BGMSoundClassPath);
	BGMSoundClass = BGMSoundClassFinder.Object;
}

bool UCPAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && !IsRunningDedicatedServer();
}

void UCPAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

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

	if (!IsValid(VolumeControlMix) || !IsValid(MasterSoundClass) || !IsValid(SFXSoundClass) || !IsValid(BGMSoundClass))
	{
		UE_LOG(LogCPAudio, Warning, TEXT("Volume SoundMix or required SoundClass asset is missing."));
		return;
	}

	if (!bVolumeMixPushed)
	{
		UGameplayStatics::PushSoundMixModifier(World, VolumeControlMix);
		bVolumeMixPushed = true;
	}

	UGameplayStatics::SetSoundMixClassOverride(World, VolumeControlMix, MasterSoundClass, MasterVolume, 1.0f, CPAudio::VolumeFadeTime, true);
	UGameplayStatics::SetSoundMixClassOverride(World, VolumeControlMix, SFXSoundClass, SFXVolume, 1.0f, CPAudio::VolumeFadeTime, true);
	UGameplayStatics::SetSoundMixClassOverride(World, VolumeControlMix, BGMSoundClass, BGMVolume, 1.0f, CPAudio::VolumeFadeTime, true);
}

void UCPAudioSubsystem::ReleaseVolumeMix()
{
	if (!bVolumeMixPushed || !IsValid(VolumeControlMix))
	{
		bVolumeMixPushed = false;
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWorld* World = GameInstance->GetWorld(); IsAudioWorld(World))
		{
			UGameplayStatics::PopSoundMixModifier(World, VolumeControlMix);
		}
	}

	bVolumeMixPushed = false;
}

bool UCPAudioSubsystem::IsAudioWorld(const UWorld* World) const
{
	return IsValid(World) && World->IsGameWorld() && World->GetGameInstance() == GetGameInstance() && World->GetNetMode() != NM_DedicatedServer;
}
