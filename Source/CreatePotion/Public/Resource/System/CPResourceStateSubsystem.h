#pragma once

#include "CoreMinimal.h"
#include "Resource/CPResourceType.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPResourceStateSubsystem.generated.h"

// 채집물 상태 관리를 위한 서브시스템
UCLASS()
class CREATEPOTION_API UCPResourceStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 상태 Get / 없으면 생성
	FCPResourceNodeState& GetOrCreateState(const FCPResourceNodeKey& Key);
	
	// 상태 검색
	const FCPResourceNodeState* FindState(const FCPResourceNodeKey& Key) const;
	
	// 채집 시(Pool 반환 및 Generation 갱신)
	void MarkHarvested(const FCPResourceNodeKey& Key, double CurrentTime, double RespawnDuration);
	
	// 리스폰 검사
	bool IsReady(const FCPResourceNodeKey& Key, double CurrentTime) const;
	
private:
	// 키 상태 맵핑
	TMap<FCPResourceNodeKey, FCPResourceNodeState> NodeStates;
};
