// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPAudioSubsystem.generated.h"

class USoundClass;
class USoundMix;
class UWorld;

/**
 * 게임 인스턴스 동안 유지되는 프로젝트 전역 사용자 볼륨을 관리합니다.
 *
 * SoundMix와 SoundClass를 통해 프로젝트 전체 오디오에
 * MasterVolume과 각 카테고리 볼륨을 즉시 반영합니다.
 * 값은 게임 인스턴스 수명 동안만 유지되며 디스크에는 자동 저장되지 않습니다.
 */
UCLASS()
class CREATEPOTION_API UCPAudioSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|Volume")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|Volume")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|Volume")
	void SetBGMVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|Volume")
	void SetUIVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|Volume")
	void SetAmbienceVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "CP|Audio|Volume")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Audio|Volume")
	float GetSFXVolume() const { return SFXVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Audio|Volume")
	float GetBGMVolume() const { return BGMVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Audio|Volume")
	float GetUIVolume() const { return UIVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Audio|Volume")
	float GetAmbienceVolume() const { return AmbienceVolume; }

private:
	void HandlePostLoadMap(UWorld* LoadedWorld);
	void ApplyVolumeMixForCurrentWorld();
	void ApplyVolumeMix(UWorld* World);
	void ReleaseVolumeMix();
	bool IsAudioWorld(const UWorld* World) const;

private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "CP|Audio|Volume", meta = (AllowPrivateAccess = "true"))
	float MasterVolume = 1.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "CP|Audio|Volume", meta = (AllowPrivateAccess = "true"))
	float SFXVolume = 1.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "CP|Audio|Volume", meta = (AllowPrivateAccess = "true"))
	float BGMVolume = 1.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "CP|Audio|Volume", meta = (AllowPrivateAccess = "true"))
	float UIVolume = 1.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "CP|Audio|Volume", meta = (AllowPrivateAccess = "true"))
	float AmbienceVolume = 1.0f;

	UPROPERTY()
	TObjectPtr<USoundMix> VolumeControlMix;

	UPROPERTY()
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY()
	TObjectPtr<USoundClass> SFXSoundClass;

	UPROPERTY()
	TObjectPtr<USoundClass> BGMSoundClass;

	UPROPERTY()
	TObjectPtr<USoundClass> UISoundClass;

	UPROPERTY()
	TObjectPtr<USoundClass> AmbienceSoundClass;

	float VolumeFadeTime = 0.05f;
	bool bVolumeMixPushed = false;
	uint32 VolumeMixAudioDeviceId = MAX_uint32;
	FDelegateHandle PostLoadMapHandle;
};
