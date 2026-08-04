#include "Resource/System/CPResourceStateSubsystem.h"

FCPResourceNodeState& UCPResourceStateSubsystem::GetOrCreateState(const FCPResourceNodeKey& Key)
{
	return NodeStates.FindOrAdd(Key);
}

const FCPResourceNodeState* UCPResourceStateSubsystem::FindState(const FCPResourceNodeKey& Key) const
{
	return NodeStates.Find(Key);
}

void UCPResourceStateSubsystem::MarkHarvested(const FCPResourceNodeKey& Key, double CurrentTime, double RespawnDuration)
{
	FCPResourceNodeState& State = NodeStates.FindOrAdd(Key);
	
	++State.Generation;
	
	State.RespawnAt = CurrentTime + RespawnDuration;
}

bool UCPResourceStateSubsystem::IsReady(const FCPResourceNodeKey& Key, double CurrentTime) const
{
	const FCPResourceNodeState* State = NodeStates.Find(Key);
	
	if (!State)
	{
		return true;
	}
	
	return CurrentTime >= State->RespawnAt;
}
