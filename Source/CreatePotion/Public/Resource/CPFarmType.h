#pragma once

#include "CoreMinimal.h"
#include "CPFarmType.generated.h"

class UCPFarmCropDefinition;

// 성장 단계: 씨앗(1단계) / 중간(2단계) / 성장 완료(3단계)
UENUM(BlueprintType)
enum class ECPCropGrowthStage : uint8
{
	None,
	Seed,
	Intermediate,
	Mature
};

// 농작물 인스턴스
USTRUCT(BlueprintType)
struct FCPFarmCropInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<UCPFarmCropDefinition> Definition = nullptr;
	
	// 성장 시간
	UPROPERTY()
	float GrowthMinutes = 0.f;
	
public:
	// 초기화
	void Initialize(UCPFarmCropDefinition* InDefinition);
	
	// 유효성 검사
	bool IsValid() const;
	
	// 농작물 정보 리셋
	void Reset();
	
	// 성장률 get
	float GetGrowthRatio() const;
	
	// 성장 단계 get
	ECPCropGrowthStage GetGrowthStage() const;
	
	// 성장 완료 여부 반환
	bool IsMature() const;
};

// 농지 한 칸 상태
USTRUCT(BlueprintType)
struct FCPFarmTileState
{
	GENERATED_BODY()
	
public:
	// 농지 수분 상태
	UPROPERTY()
	float SoilMoisture = 0.f;
	
	UPROPERTY()
	FCPFarmCropInstance Crop;
	
public:
	bool HasCrop() const;
};

// 농지 식별
USTRUCT(BlueprintType)
struct FCPFarmTileKey
{
	GENERATED_BODY()
	
public:
	// 개별 농장 Id
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName FarmId;
	
	// 농장 개별 칸. 농지는 2차원 그리드 형태가 될 예정이라, FIntPoint 활용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FIntPoint Coordinate = FIntPoint::ZeroValue;
	
	// 농지 칸 일치 여부 확인
	bool operator==(const FCPFarmTileKey& Other) const;
	
	// 해시 값 변환
	friend uint32 GetTypeHash(const FCPFarmTileKey& Key);
};

