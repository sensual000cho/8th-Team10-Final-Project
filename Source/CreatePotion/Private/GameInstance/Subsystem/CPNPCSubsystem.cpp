#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"
#include "NPC/CPBaseNPC.h"

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

	NotifyEffectApplied(NPCId, EffectTag, ExpiresAt, Magnitude);
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
			NotifyEffectExpired(NPCId, Tag);
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
		NotifyEffectExpired(NPCId, EffectTag);

		if (Found->ActiveEffects.Num() == 0)
		{
			NPCStateRepository.Remove(NPCId);
		}
	}
}

void UCPNPCSubsystem::RegisterActiveNPC(FName NPCId, ACPBaseNPC* NPC)
{
	if (NPCId.IsNone() || !IsValid(NPC)) return;
	ActiveNPCRegistry.Add(NPCId, NPC);
}

void UCPNPCSubsystem::UnregisterActiveNPC(FName NPCId, ACPBaseNPC* NPC)
{
	if (NPCId.IsNone()) return;

	if (TWeakObjectPtr<ACPBaseNPC>* Found = ActiveNPCRegistry.Find(NPCId))
	{
		if (Found->Get() == NPC)
		{
			ActiveNPCRegistry.Remove(NPCId);
		}
	}
}

void UCPNPCSubsystem::NotifyEffectApplied(FName NPCId, FGameplayTag EffectTag, int64 ExpiresAtWorldMinute, float Magnitude)
{
	if (TWeakObjectPtr<ACPBaseNPC>* Found = ActiveNPCRegistry.Find(NPCId))
	{
		if (ACPBaseNPC* NPC = Found->Get())
		{
			NPC->HandlePotionEffectApplied(EffectTag, ExpiresAtWorldMinute, Magnitude);
		}
	}

	OnNPCEffectApplied.Broadcast(NPCId, EffectTag, ExpiresAtWorldMinute, Magnitude);
}

void UCPNPCSubsystem::NotifyEffectExpired(FName NPCId, FGameplayTag EffectTag)
{
	if (TWeakObjectPtr<ACPBaseNPC>* Found = ActiveNPCRegistry.Find(NPCId))
	{
		if (ACPBaseNPC* NPC = Found->Get())
		{
			NPC->HandlePotionEffectExpired(EffectTag);
		}
	}

	OnNPCEffectExpired.Broadcast(NPCId, EffectTag);
}