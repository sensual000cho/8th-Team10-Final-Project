// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameInstance/Subsystem/CPAudioSubsystem.h"

#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

bool UCPAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer) && !IsRunningDedicatedServer();
}

void UCPAudioSubsystem::Deinitialize()
{
	ReleaseBGMComponent();

	for (const TPair<TObjectPtr<UAudioComponent>, float>& Entry : ManagedSFXComponents)
	{
		if (IsValid(Entry.Key))
		{
			Entry.Key->OnAudioFinishedNative.RemoveAll(this);
			Entry.Key->Stop();
		}
	}
	ManagedSFXComponents.Reset();

	Super::Deinitialize();
}

void UCPAudioSubsystem::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyBGMVolume();
	ApplySFXVolume();
}

void UCPAudioSubsystem::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplySFXVolume();
}

void UCPAudioSubsystem::SetBGMVolume(float Volume)
{
	BGMVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplyBGMVolume();
}

UAudioComponent* UCPAudioSubsystem::PlayBGM(USoundBase* Sound, float FadeInDuration, float VolumeMultiplier, float StartTime)
{
	if (!IsValid(Sound))
	{
		return nullptr;
	}

	CurrentBGMBaseVolume = FMath::Max(0.0f, VolumeMultiplier);

	if (IsValid(BGMComponent) && CurrentBGM == Sound && BGMComponent->IsPlaying())
	{
		ApplyBGMVolume();
		return BGMComponent;
	}

	if (!IsValid(BGMComponent))
	{
		BGMComponent = UGameplayStatics::CreateSound2D(GetGameInstance(), Sound, 1.0f, 1.0f, FMath::Max(0.0f, StartTime), nullptr, true, false);

		if (!IsValid(BGMComponent))
		{
			return nullptr;
		}

		BGMComponent->bIsUISound = true;
	}
	else
	{
		BGMComponent->Stop();
		BGMComponent->SetSound(Sound);
	}

	CurrentBGM = Sound;
	ApplyBGMVolume();
	BGMComponent->FadeIn(FMath::Max(0.0f, FadeInDuration), 1.0f, FMath::Max(0.0f, StartTime));

	return BGMComponent;
}

void UCPAudioSubsystem::StopBGM(float FadeOutDuration)
{
	CurrentBGM = nullptr;

	if (!IsValid(BGMComponent) || !BGMComponent->IsPlaying())
	{
		return;
	}

	if (FadeOutDuration > 0.0f)
	{
		BGMComponent->FadeOut(FadeOutDuration, 0.0f);
	}
	else
	{
		BGMComponent->Stop();
	}
}

UAudioComponent* UCPAudioSubsystem::PlaySFX2D(USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier, float StartTime, bool bIsUISound)
{
	if (!IsValid(Sound))
	{
		return nullptr;
	}

	const float BaseVolume = FMath::Max(0.0f, VolumeMultiplier);
	UAudioComponent* AudioComponent = UGameplayStatics::CreateSound2D(GetGameInstance(), Sound, BaseVolume * MasterVolume * SFXVolume, FMath::Max(0.0f, PitchMultiplier), FMath::Max(0.0f, StartTime), nullptr, false, true);

	if (!IsValid(AudioComponent))
	{
		return nullptr;
	}

	AudioComponent->bIsUISound = bIsUISound;
	ManagedSFXComponents.Add(AudioComponent, BaseVolume);
	AudioComponent->OnAudioFinishedNative.AddUObject(this, &ThisClass::HandleSFXFinished);
	AudioComponent->Play(FMath::Max(0.0f, StartTime));

	return AudioComponent;
}

UAudioComponent* UCPAudioSubsystem::PlaySFXAtLocation(USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier, float StartTime)
{
	if (!IsValid(Sound))
	{
		return nullptr;
	}

	const float BaseVolume = FMath::Max(0.0f, VolumeMultiplier);
	UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetGameInstance(), Sound, Location, FRotator::ZeroRotator, BaseVolume * MasterVolume * SFXVolume, FMath::Max(0.0f, PitchMultiplier), FMath::Max(0.0f, StartTime));

	if (!IsValid(AudioComponent))
	{
		return nullptr;
	}

	ManagedSFXComponents.Add(AudioComponent, BaseVolume);
	AudioComponent->OnAudioFinishedNative.AddUObject(this, &ThisClass::HandleSFXFinished);

	return AudioComponent;
}

void UCPAudioSubsystem::ApplyBGMVolume()
{
	if (IsValid(BGMComponent))
	{
		BGMComponent->SetVolumeMultiplier(CurrentBGMBaseVolume * MasterVolume * BGMVolume);
	}
}

void UCPAudioSubsystem::ApplySFXVolume()
{
	for (auto It = ManagedSFXComponents.CreateIterator(); It; ++It)
	{
		if (!IsValid(It.Key()))
		{
			It.RemoveCurrent();
			continue;
		}

		It.Key()->SetVolumeMultiplier(It.Value() * MasterVolume * SFXVolume);
	}
}

void UCPAudioSubsystem::HandleSFXFinished(UAudioComponent* FinishedComponent)
{
	if (IsValid(FinishedComponent))
	{
		FinishedComponent->OnAudioFinishedNative.RemoveAll(this);
	}

	ManagedSFXComponents.Remove(FinishedComponent);
}

void UCPAudioSubsystem::ReleaseBGMComponent()
{
	CurrentBGM = nullptr;

	if (!IsValid(BGMComponent))
	{
		BGMComponent = nullptr;
		return;
	}

	BGMComponent->Stop();
	BGMComponent->DestroyComponent();
	BGMComponent = nullptr;
}
