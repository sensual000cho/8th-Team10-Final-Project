#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h" 
#include "CPNPCSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FCPActiveEffectInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	FGameplayTag EffectTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	int64 AppliedAtWorldMinute = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	int64 ExpiresAtWorldMinute = 0;
};

USTRUCT(BlueprintType)
struct FCPNPCEffectSaveData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	TMap<FGameplayTag, FCPActiveEffectInfo> ActiveEffects;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCEffectExpired, FName, NPCId, FGameplayTag, ExpiredTag);

UCLASS()
class CREATEPOTION_API UCPNPCSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 시간 매니저의 1분 틱을 수신하는 콜백 함수
	UFUNCTION()
	void OnWorldClockMinuteTick(int64 CurrentWorldMinute);

	// 지속 포션 효과 등록 (거대화 등)
	UFUNCTION(BlueprintCallable, Category = "NPC|Effect")
	void RegisterNPCEffect(FName NPCId, FGameplayTag EffectTag, int64 DurationWorldMinutes);

	// NPC 레벨 재진입 시 상태 복원용 데이터 조회
	UFUNCTION(BlueprintCallable, Category = "NPC|Effect")
	bool GetNPCEffectData(FName NPCId, FCPNPCEffectSaveData& OutSaveData) const;

	// 특정 효과 강제 해제
	UFUNCTION(BlueprintCallable, Category = "NPC|Effect")
	void ClearNPCEffect(FName NPCId, FGameplayTag EffectTag);

	// 디버그 테스트용
	UFUNCTION(BlueprintCallable, Category = "NPC|EffectDebug")
	void DebugAddNPCEffect(FName NPCId, FGameplayTag EffectTag, int64 DurationWorldMinutes);

	UPROPERTY(BlueprintAssignable, Category = "NPC|Effect")
	FOnNPCEffectExpired OnNPCEffectExpired;

private:
	// NPCId -> 해당 NPC의 효과 목록 저장소
	UPROPERTY()
	TMap<FName, FCPNPCEffectSaveData> NPCStateRepository;

	int64 CachedCurrentWorldMinute = 0;
};