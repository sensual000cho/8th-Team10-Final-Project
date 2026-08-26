#include "Resource/System/CPResourceStateSubsystem.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"

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
	
	OnResourceNodeHarvested.Broadcast(Key);
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
