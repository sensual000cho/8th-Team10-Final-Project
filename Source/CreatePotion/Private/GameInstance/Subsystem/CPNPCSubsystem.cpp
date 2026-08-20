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

void UCPNPCSubsystem::RegisterNPCEffect(FName NPCId, FGameplayTag EffectTag, int64 DurationWorldMinutes)
{
	if (NPCId.IsNone() || !EffectTag.IsValid()) return;

	int64 AppliedAt = CachedCurrentWorldMinute;
	int64 ExpiresAt = AppliedAt + DurationWorldMinutes;

	FCPActiveEffectInfo Info;
	Info.EffectTag = EffectTag;
	Info.AppliedAtWorldMinute = AppliedAt;
	Info.ExpiresAtWorldMinute = ExpiresAt;

	FCPNPCEffectSaveData& SaveData = NPCStateRepository.FindOrAdd(NPCId);
	SaveData.ActiveEffects.Add(EffectTag, Info);

	FString DisplayMsg = FString::Printf(TEXT("[%s]에게 %s 효과 적용됨! (만료 예정: %lld분 / 지속: %lld분)"),
		*NPCId.ToString(), *EffectTag.GetTagName().ToString(), ExpiresAt, DurationWorldMinutes);

	UE_LOG(LogTemp, Log, TEXT("%s"), *DisplayMsg);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, DisplayMsg);
	}
}

void UCPNPCSubsystem::OnWorldClockMinuteTick(int64 CurrentWorldMinute)
{
	CachedCurrentWorldMinute = CurrentWorldMinute;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.5f, FColor::Green, FString::Printf(TEXT("월드 시간: %lld분"), CurrentWorldMinute));
	}

	TArray<FName> EmptyNPCIds;

	for (auto& Pair : NPCStateRepository)
	{
		FName NPCId = Pair.Key;
		FCPNPCEffectSaveData& SaveData = Pair.Value;
		TArray<FGameplayTag> ExpiredTags;

		for (auto& EffectPair : SaveData.ActiveEffects)
		{
			int64 LeftTime = EffectPair.Value.ExpiresAtWorldMinute - CurrentWorldMinute;

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
					FString::Printf(TEXT("[%s] %s 남은 시간: %lld분"),
						*NPCId.ToString(), *EffectPair.Key.GetTagName().ToString(), LeftTime));
			}

			if (CurrentWorldMinute >= EffectPair.Value.ExpiresAtWorldMinute)
			{
				ExpiredTags.Add(EffectPair.Key);
			}
		}

		for (const FGameplayTag& Tag : ExpiredTags)
		{
			SaveData.ActiveEffects.Remove(Tag);
			OnNPCEffectExpired.Broadcast(NPCId, Tag);

			FString ExpiredMsg = FString::Printf(TEXT("[%s]의 %s 효과 만료!"), *NPCId.ToString(), *Tag.GetTagName().ToString());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *ExpiredMsg);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ExpiredMsg);
			}
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