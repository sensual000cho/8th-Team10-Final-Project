#include "GameInstance/Subsystem/CPTimeSubsystem.h"

int32 UCPTimeSubsystem::GetCurrentDay() const
{
	// 하루 = 24시간 = 1440분
	return TotalWorldMinutes / 1440;
}

int64 UCPTimeSubsystem::GetTotalWorldMinutes() const
{
	return TotalWorldMinutes;
}

ECPTimePhase UCPTimeSubsystem::GetCurrentPhase() const
{
	const int32 Minutes = static_cast<int32>(TotalWorldMinutes) % 1440;
	
	constexpr int32 MorningStart = 6 * 60;
	constexpr int32 DayStart = 12 * 60;
	constexpr int32 NightStart = 18 * 60;
	
	if (Minutes < MorningStart)
	{
		return ECPTimePhase::Dawn;
	}
	
	else if (Minutes < DayStart)
	{
		return ECPTimePhase::Morning;
	}
	
	else if (Minutes < NightStart)
	{
		return ECPTimePhase::Day;
	}

	else
	{
		return ECPTimePhase::Night;
	}
}

void UCPTimeSubsystem::AdvanceTime(int64 Amount)
{
	const int32 OldDay = GetCurrentDay();
	const ECPTimePhase OldPhase = GetCurrentPhase();
	
	TotalWorldMinutes += Amount;
	
	const int32 NewDay = GetCurrentDay();
	const ECPTimePhase NewPhase = GetCurrentPhase();
	
	OnTimeChanged.Broadcast(TotalWorldMinutes);
	if (OldDay != NewDay) OnDayChanged.Broadcast(NewDay);
	if (OldPhase != NewPhase) OnTimePhaseChanged.Broadcast(NewPhase);
	
	// 시간 체크용
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("%lld"), TotalWorldMinutes));
}

void UCPTimeSubsystem::StartTime()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	FTimerManager& TimerManager = GI->GetTimerManager();
	
	if (TimerManager.IsTimerActive(TimeTickHandle)) return;
	
	if (TimerManager.IsTimerPaused(TimeTickHandle))
	{
		TimerManager.UnPauseTimer(TimeTickHandle);
		return;
	}
	
	TimerManager.SetTimer(
		TimeTickHandle,
		this,
		&UCPTimeSubsystem::HandleTimeTick,
		1.f,
		true
	);
}

void UCPTimeSubsystem::PauseTime()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	GI->GetTimerManager().PauseTimer(TimeTickHandle);
}

void UCPTimeSubsystem::ResumeTime()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	GI->GetTimerManager().UnPauseTimer(TimeTickHandle);
}

void UCPTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCPTimeSubsystem::Deinitialize()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		GI->GetTimerManager().ClearTimer(TimeTickHandle);
	}
	
	Super::Deinitialize();
}

void UCPTimeSubsystem::HandleTimeTick()
{
	AdvanceTime(1.0);
}
