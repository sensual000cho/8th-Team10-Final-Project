#pragma once

#include "CoreMinimal.h"
#include "Resource/CPFarmType.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPFarmSubsystem.generated.h"

// 농사 서브시스템
UCLASS()
class CREATEPOTION_API UCPFarmSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// 농지 등록
	bool RegisterTile(const FCPFarmTileKey& Key);
	
	// 상태 조회
	const FCPFarmTileState* FindTileState(const FCPFarmTileKey& Key) const;
	
	// 심기
	bool PlantCrop(const FCPFarmTileKey& Key, UCPFarmCropDefinition* CropDefinition);
	
	// 물주기
	bool WaterTile(const FCPFarmTileKey& Key);
	
	// 농작물 상태 경과(시간)
	UFUNCTION()
	void AdvanceFarmTime(int64 DeltaGameMinutes);
	
private:
	// 상태 조회 (내부 수정용)
	FCPFarmTileState* FindMutableTileState(const FCPFarmTileKey& Key);
	
private:
	UPROPERTY()
	TMap<FCPFarmTileKey, FCPFarmTileState> FarmTileStates;
	
	// 수분량 / 수분량 100% -> 0% 걸리는 시간
	static constexpr float MaxSoilMoisture = 100.f;
	static constexpr float FullMoistureDurationMinutes = 60.f;
};
