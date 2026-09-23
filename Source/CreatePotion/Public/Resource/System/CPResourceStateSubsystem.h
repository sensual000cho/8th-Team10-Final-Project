#pragma once

#include "CoreMinimal.h"
#include "Resource/CPResourceType.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPResourceStateSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnResourceNodeHarvested, const FCPResourceNodeKey&);

// 채집물 상태 관리를 위한 서브시스템
UCLASS()
class CREATEPOTION_API UCPResourceStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// 게임 세션 시드 Getter
	int32 GetSessionSeed() const { return SessionSeed; }
	
	// 상태 Get / 없으면 생성
	FCPResourceNodeState& GetOrCreateState(const FCPResourceNodeKey& Key);
	
	// 상태 검색
	const FCPResourceNodeState* FindState(const FCPResourceNodeKey& Key) const;
	
	// 채집 시(Pool 반환 및 Generation 갱신)
	void MarkHarvested(const FCPResourceNodeKey& Key, double RespawnDuration);
	
	// 나무 덩굴 제거 시
	bool MarkVineCleared(const FCPResourceNodeKey& Key);
	
	// 리스폰 검사
	bool IsReady(const FCPResourceNodeKey& Key) const;
	
	FOnResourceNodeHarvested OnResourceNodeHarvested;
	
private:
	// 키 상태 맵핑
	TMap<FCPResourceNodeKey, FCPResourceNodeState> NodeStates;
	
	// 게임 세션 시드
	int32 SessionSeed = 0;
};
