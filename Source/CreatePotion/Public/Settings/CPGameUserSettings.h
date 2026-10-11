// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "CPGameUserSettings.generated.h"

// 로컬 사용자 환경설정을 GameUserSettings.ini에 제장
 
// 해상도, 창 모드 등은 UGameUserSettings의 기본 기능을 사용
//카테고리별 볼륨만 이 클래스에서 확장
 
UCLASS(Config = GameUserSettings)
class CREATEPOTION_API UCPGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	virtual void ApplyNonResolutionSettings() override;
	virtual void LoadSettings(bool bForceReload = false) override;
	virtual void SetToDefaults() override;
	virtual void ValidateSettings() override;

	UFUNCTION(BlueprintPure, Category = "CP|Settings", meta = (DisplayName = "Get CP Game User Settings"))
	static UCPGameUserSettings* GetCPGameUserSettings();

	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Audio")
	void ApplyAudioSettings() const;

	// 볼륨만 적용한 뒤 GameUserSettings.ini에 저장합니다. 해상도와 창 모드는 적용하지 않습니다.
	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Audio", meta = (DisplayName = "Apply And Save Audio Settings"))
	void ApplyAndSaveAudioSettings();

	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Audio")
	void SetSFXVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Audio")
	void SetBGMVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Audio")
	void SetUIVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Audio")
	void SetAmbienceVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Screen")
	void SetGraphicsLevel(int32 InLevel); 
	
	UFUNCTION(BlueprintPure, Category = "CP|Settings|Audio")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Settings|Audio")
	float GetSFXVolume() const { return SFXVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Settings|Audio")
	float GetBGMVolume() const { return BGMVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Settings|Audio")
	float GetUIVolume() const { return UIVolume; }

	UFUNCTION(BlueprintPure, Category = "CP|Settings|Audio")
	float GetAmbienceVolume() const { return AmbienceVolume; }

	UFUNCTION(BlueprintCallable, Category = "CP|Settings|Screen")
	int32 GetGraphicsLevel() const { return GraphicsLevel; }
private:
	void ClampAudioVolumes();

private:
	UPROPERTY(Config)
	float MasterVolume = 1.0f;

	UPROPERTY(Config)
	float SFXVolume = 1.0f;

	UPROPERTY(Config)
	float BGMVolume = 1.0f;

	UPROPERTY(Config)
	float UIVolume = 1.0f;

	UPROPERTY(Config)
	float AmbienceVolume = 1.0f;
	
	UPROPERTY(Config)
	int32 GraphicsLevel = 2;
};
