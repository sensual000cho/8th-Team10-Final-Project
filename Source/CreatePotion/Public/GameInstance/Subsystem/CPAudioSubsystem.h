// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPAudioSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

/**
 * 게임 인스턴스 동안 유지되는 오디오 재생과 사용자 볼륨을 관리합니다.
 *
 * 이 서브시스템을 통해 생성한 BGM/SFX 컴포넌트에는
 * MasterVolume과 각 카테고리 볼륨이 즉시 반영됩니다.
 * 값은 게임 인스턴스 수명 동안만 유지되며 디스크에는 자동 저장되지 않습니다.
 */
UCLASS()
class CREATEPOTION_API UCPAudioSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|Volume")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|Volume")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|Volume")
	void SetBGMVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "CP|Audio|Volume")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Audio|Volume")
	float GetSFXVolume() const { return SFXVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Audio|Volume")
	float GetBGMVolume() const { return BGMVolume; }

	/** 맵 전환에도 유지되는 비공간화 BGM을 재생합니다. */
	UFUNCTION(BlueprintCallable, Category = "CP|Audio|BGM")
	UAudioComponent* PlayBGM(USoundBase* Sound, float FadeInDuration = 1.0f, float VolumeMultiplier = 1.0f, float StartTime = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "CP|Audio|BGM")
	void StopBGM(float FadeOutDuration = 1.0f);

	/** 서브시스템의 Master/SFX 볼륨을 따르는 비공간화 효과음을 재생합니다. */
	UFUNCTION(BlueprintCallable, Category = "CP|Audio|SFX")
	UAudioComponent* PlaySFX2D(USoundBase* Sound, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, float StartTime = 0.0f, bool bIsUISound = false);

	/** 서브시스템의 Master/SFX 볼륨을 따르는 공간화 효과음을 재생합니다. */
	UFUNCTION(BlueprintCallable, Category = "CP|Audio|SFX")
	UAudioComponent* PlaySFXAtLocation(USoundBase* Sound, FVector Location, float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f, float StartTime = 0.0f);

private:
	void ApplyBGMVolume();
	void ApplySFXVolume();
	void HandleSFXFinished(UAudioComponent* FinishedComponent);
	void ReleaseBGMComponent();

private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "CP|Audio|Volume", meta = (AllowPrivateAccess = "true"))
	float MasterVolume = 1.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "CP|Audio|Volume", meta = (AllowPrivateAccess = "true"))
	float SFXVolume = 1.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "CP|Audio|Volume", meta = (AllowPrivateAccess = "true"))
	float BGMVolume = 1.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BGMComponent;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentBGM;

	UPROPERTY(Transient)
	float CurrentBGMBaseVolume = 1.0f;

	/** 컴포넌트별 기본 볼륨을 보관해 슬라이더 변경 시 재계산합니다. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UAudioComponent>, float> ManagedSFXComponents;
};
