#include "Resource/System/CPResourceStateSubsystem.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"

void UCPResourceStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	SessionSeed = static_cast<int32>(FGuid::NewGuid().A & 0x7FFFFFFF);
}

FCPResourceSaveData UCPResourceStateSubsystem::GetSaveData() const
{
	FCPResourceSaveData SaveData;
	SaveData.NodeStates = NodeStates;
	SaveData.SessionSeed = SessionSeed;
	
	return SaveData;
}

void UCPResourceStateSubsystem::LoadSaveData(const FCPResourceSaveData& SaveData)
{
	NodeStates = SaveData.NodeStates;
	SessionSeed = SaveData.SessionSeed;
}

FCPResourceNodeState& UCPResourceStateSubsystem::GetOrCreateState(const FCPResourceNodeKey& Key)
{
	return NodeStates.FindOrAdd(Key);
}

const FCPResourceNodeState* UCPResourceStateSubsystem::FindState(const FCPResourceNodeKey& Key) const
{
	return NodeStates.Find(Key);
}

void UCPResourceStateSubsystem::MarkHarvested(const FCPResourceNodeKey& Key, double RespawnDuration)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	
	UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>();
	if (!TimeSubsystem) return;
	
	FCPResourceNodeState& State = NodeStates.FindOrAdd(Key);
	
	++State.Generation;
	
	State.RespawnAt = TimeSubsystem->GetTotalWorldMinutes() + RespawnDuration;
	State.bVineCleared = false;
	
	OnResourceNodeHarvested.Broadcast(Key);
}

bool UCPResourceStateSubsystem::MarkVineCleared(const FCPResourceNodeKey& Key)
{
	if (!Key.IsValid()) return false;
	
	FCPResourceNodeState& State = NodeStates.FindOrAdd(Key);
	
	if (State.bVineCleared) return false;
	
	State.bVineCleared = true;
	
	return true;
}

bool UCPResourceStateSubsystem::IsReady(const FCPResourceNodeKey& Key) const
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return false;
	
	UCPTimeSubsystem* TimeSubsystem = GI->GetSubsystem<UCPTimeSubsystem>();
	if (!TimeSubsystem) return false;
	
	const FCPResourceNodeState* State = NodeStates.Find(Key);
	
	if (!State)
	{
		return true;
	}
	
	return TimeSubsystem->GetTotalWorldMinutes() >= State->RespawnAt;
}
