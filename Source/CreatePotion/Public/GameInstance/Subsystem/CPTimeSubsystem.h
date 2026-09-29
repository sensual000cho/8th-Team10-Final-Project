#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPTimeSubsystem.generated.h"

// 현재 시간 상태(새벽, 아침, 낮, 밤)
// 00:00 ~ 05:59 새벽
// 06:00 ~ 11:59 아침
// 12:00 ~ 17:59 낮
// 18:00 ~ 23:59 밤
UENUM(BlueprintType)
enum class ECPTimePhase : uint8
{
	Dawn,
	Morning,
	Day,
	Night
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeChanged, int64, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32, Day);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimePhaseChanged, ECPTimePhase, Phase);

// 시간 관리 서브시스템
UCLASS()
class CREATEPOTION_API UCPTimeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	int32 GetCurrentDay() const;
	int64 GetTotalWorldMinutes() const;
	UFUNCTION(BlueprintCallable)
	ECPTimePhase GetCurrentPhase() const;
	
	// Game Save
	void LoadSaveData(int64 SavedTotalWorldMinutes);
	
	FOnTimeChanged OnTimeChanged;
	FOnDayChanged OnDayChanged;
	FOnTimePhaseChanged OnTimePhaseChanged;
	
	UFUNCTION(BlueprintCallable)
	void AdvanceTime(int64 Amount);
	
	UFUNCTION(BlueprintCallable)
	void StartTime();
	
	UFUNCTION(BlueprintCallable)
	void PauseTime();
	
	UFUNCTION(BlueprintCallable)
	void ResumeTime();
	
private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION()
	void HandleTimeTick();
	
	// 총 시간
	int64 TotalWorldMinutes = 0;
	
	FTimerHandle TimeTickHandle;
};
