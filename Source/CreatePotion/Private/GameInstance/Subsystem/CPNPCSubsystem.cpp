#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"

void UCPNPCSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UCPTimeSubsystem>();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>())
		{
			TimeSubsystem->OnTimeChanged.AddDynamic(this, &UCPNPCSubsystem::OnWorldClockMinuteTick);
			CachedCurrentWorldMinute = TimeSubsystem->GetTotalWorldMinutes();
		}
	}
}

void UCPNPCSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>())
		{
			TimeSubsystem->OnTimeChanged.RemoveDynamic(this, &UCPNPCSubsystem::OnWorldClockMinuteTick);
		}
	}
	Super::Deinitialize();
}

void UCPNPCSubsystem::RegisterNPCEffect(FName NPCId, FGameplayTag EffectTag, int64 DurationWorldMinutes, float Magnitude)
{
	if (NPCId.IsNone() || !EffectTag.IsValid()) return;

	const int64 AppliedAt = CachedCurrentWorldMinute;
	const int64 ExpiresAt = AppliedAt + DurationWorldMinutes;

	FCPActiveEffectInfo Info;
	Info.EffectTag = EffectTag;
	Info.AppliedAtWorldMinute = AppliedAt;
	Info.ExpiresAtWorldMinute = ExpiresAt;
	Info.Magnitude = Magnitude;

	FCPNPCEffectSaveData& SaveData = NPCStateRepository.FindOrAdd(NPCId);
	SaveData.ActiveEffects.Add(EffectTag, Info);

	OnNPCEffectApplied.Broadcast(NPCId, EffectTag, ExpiresAt, Magnitude);
}

void UCPNPCSubsystem::OnWorldClockMinuteTick(int64 CurrentWorldMinute)
{
	CachedCurrentWorldMinute = CurrentWorldMinute;

	TArray<FName> EmptyNPCIds;

	for (auto& Pair : NPCStateRepository)
	{
		FName NPCId = Pair.Key;
		FCPNPCEffectSaveData& SaveData = Pair.Value;
		TArray<FGameplayTag> ExpiredTags;

		for (auto& EffectPair : SaveData.ActiveEffects)
		{
			if (CurrentWorldMinute >= EffectPair.Value.ExpiresAtWorldMinute)
			{
				ExpiredTags.Add(EffectPair.Key);
			}
		}

		for (const FGameplayTag& Tag : ExpiredTags)
		{
			SaveData.ActiveEffects.Remove(Tag);
			OnNPCEffectExpired.Broadcast(NPCId, Tag);
		}

		if (SaveData.ActiveEffects.Num() == 0)
		{
			EmptyNPCIds.Add(NPCId);
		}
	}

	for (const FName& Id : EmptyNPCIds)
	{
		NPCStateRepository.Remove(Id);
	}
}

bool UCPNPCSubsystem::GetNPCEffectData(FName NPCId, FCPNPCEffectSaveData& OutSaveData) const
{
	if (const FCPNPCEffectSaveData* Found = NPCStateRepository.Find(NPCId))
	{
		OutSaveData = *Found;
		return true;
	}
	return false;
}

void UCPNPCSubsystem::ClearNPCEffect(FName NPCId, FGameplayTag EffectTag)
{
	if (FCPNPCEffectSaveData* Found = NPCStateRepository.Find(NPCId))
	{
		Found->ActiveEffects.Remove(EffectTag);
		OnNPCEffectExpired.Broadcast(NPCId, EffectTag);

		if (Found->ActiveEffects.Num() == 0)
		{
			NPCStateRepository.Remove(NPCId);
		}
	}
}

void UCPNPCSubsystem::DebugAddNPCEffect(FName NPCId, FGameplayTag EffectTag, int64 DurationWorldMinutes)
{
	RegisterNPCEffect(NPCId, EffectTag, DurationWorldMinutes);
}