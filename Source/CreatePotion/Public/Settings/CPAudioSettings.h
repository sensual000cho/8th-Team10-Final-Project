// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CPAudioSettings.generated.h"

class USoundClass;
class USoundMix;

/**
 * 프로젝트 전역 볼륨 제어에 사용할 오디오 에셋 설정입니다.
 * Project Settings > Game > CreatePotion Audio Settings에서 지정합니다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "CreatePotion Audio Settings"))
class CREATEPOTION_API UCPAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Volume")
	TSoftObjectPtr<USoundMix> VolumeControlMix;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Volume")
	TSoftObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Volume")
	TSoftObjectPtr<USoundClass> SFXSoundClass;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Volume")
	TSoftObjectPtr<USoundClass> BGMSoundClass;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Volume")
	TSoftObjectPtr<USoundClass> UISoundClass;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Volume")
	TSoftObjectPtr<USoundClass> AmbienceSoundClass;

	UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category = "Volume", meta = (ClampMin = "0.0", Units = "s"))
	float VolumeFadeTime = 0.05f;
};
