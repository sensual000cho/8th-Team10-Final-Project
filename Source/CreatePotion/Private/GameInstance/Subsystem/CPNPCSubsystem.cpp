#include "GameInstance/Subsystem/CPNPCSubsystem.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/Components/CPNPCMoveComponent.h"

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

FCPNPCSaveData UCPNPCSubsystem::GetSaveData()
{
	for (const TPair<FName, TArray<TWeakObjectPtr<ACPBaseNPC>>>& Pair : ActiveNPCRegistry){
		for (const TWeakObjectPtr<ACPBaseNPC>& WeakNPC : Pair.Value){
			ACPBaseNPC* NPC = WeakNPC.Get();
			if (!NPC) continue;
			
			UCPNPCMoveComponent* MoveComponent = NPC->FindComponentByClass<UCPNPCMoveComponent>();
			if (!MoveComponent) continue;
			
			MoveComponent->SaveMovementState();
		}
	}
	
	FCPNPCSaveData SaveData;
	SaveData.NPCEffects = NPCStateRepository;
	SaveData.NPCMovements = NPCMovementRepository;

	return SaveData;
}

void UCPNPCSubsystem::LoadSaveData(const FCPNPCSaveData& SaveData)
{
	NPCStateRepository = SaveData.NPCEffects;
	NPCMovementRepository = SaveData.NPCMovements;
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

	TArray<TWeakObjectPtr<ACPBaseNPC>>& NPCList = ActiveNPCRegistry.FindOrAdd(NPCId);
	NPCList.AddUnique(NPC);
}

void UCPNPCSubsystem::UnregisterActiveNPC(FName NPCId, ACPBaseNPC* NPC)
{
	if (NPCId.IsNone()) return;

	if (TArray<TWeakObjectPtr<ACPBaseNPC>>* NPCList = ActiveNPCRegistry.Find(NPCId))
	{
		NPCList->RemoveAll([NPC](const TWeakObjectPtr<ACPBaseNPC>& WeakNPC)
			{
				return !WeakNPC.IsValid() || WeakNPC.Get() == NPC;
			});

		if (NPCList->Num() == 0)
		{
			ActiveNPCRegistry.Remove(NPCId);
		}
	}
}

void UCPNPCSubsystem::SaveNPCMovementData(FName NPCId, const FCPNPCMovementSaveData& MovementData)
{
	if (NPCId.IsNone()) return;
	NPCMovementRepository.Add(NPCId, MovementData);
}

bool UCPNPCSubsystem::GetNPCMovementData(FName NPCId, FCPNPCMovementSaveData& OutMovementData) const
{
	if (const FCPNPCMovementSaveData* Found = NPCMovementRepository.Find(NPCId))
	{
		OutMovementData = *Found;
		return true;
	}
	return false;
}

void UCPNPCSubsystem::SetNPCActiveStateById(FName NPCId, bool bActive, ACPBaseNPC* IgnoreNPC)
{
	if (NPCId.IsNone()) return;

	if (TArray<TWeakObjectPtr<ACPBaseNPC>>* NPCList = ActiveNPCRegistry.Find(NPCId))
	{
		for (const TWeakObjectPtr<ACPBaseNPC>& WeakNPC : *NPCList)
		{
			if (ACPBaseNPC* NPC = WeakNPC.Get())
			{
				if (NPC != IgnoreNPC)
				{
					NPC->SetNPCActiveState(bActive);
				}
			}
		}
	}
}

void UCPNPCSubsystem::NotifyEffectApplied(FName NPCId, FGameplayTag EffectTag, int64 ExpiresAtWorldMinute, float Magnitude)
{
	if (TArray<TWeakObjectPtr<ACPBaseNPC>>* NPCList = ActiveNPCRegistry.Find(NPCId))
	{
		for (const TWeakObjectPtr<ACPBaseNPC>& WeakNPC : *NPCList)
		{
			if (ACPBaseNPC* NPC = WeakNPC.Get())
			{
				NPC->HandlePotionEffectApplied(EffectTag, ExpiresAtWorldMinute, Magnitude);
			}
		}
	}

	OnNPCEffectApplied.Broadcast(NPCId, EffectTag, ExpiresAtWorldMinute, Magnitude);
}

void UCPNPCSubsystem::NotifyEffectExpired(FName NPCId, FGameplayTag EffectTag)
{
	if (TArray<TWeakObjectPtr<ACPBaseNPC>>* NPCList = ActiveNPCRegistry.Find(NPCId))
	{
		for (const TWeakObjectPtr<ACPBaseNPC>& WeakNPC : *NPCList)
		{
			if (ACPBaseNPC* NPC = WeakNPC.Get())
			{
				NPC->HandlePotionEffectExpired(EffectTag);
			}
		}
	}

	OnNPCEffectExpired.Broadcast(NPCId, EffectTag);
}