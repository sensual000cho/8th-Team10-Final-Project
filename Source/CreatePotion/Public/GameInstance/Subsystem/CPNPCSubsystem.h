#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/NPC/CPNPCTargetData.h"
#include "CPNPCSubsystem.generated.h"

class ACPBaseNPC;

USTRUCT(BlueprintType)
struct FCPNPCMovementSaveData
{
	GENERATED_BODY()

	// 레벨을 벗어날 당시의 실제 월드 좌표
	UPROPERTY(BlueprintReadOnly)
	FVector SavedLocation = FVector::ZeroVector;

	// 레벨을 벗어날 당시의 회전값
	UPROPERTY(BlueprintReadOnly)
	FRotator SavedRotation = FRotator::ZeroRotator;

	// 다시 진입했을 때 이동해야 할 다음 타겟 인덱스
	UPROPERTY(BlueprintReadOnly)
	int32 NextPointIndex = 0;

	// 한 번이라도 데이터가 기록된 적이 있는지 플래그
	UPROPERTY(BlueprintReadOnly)
	bool bHasSavedTransform = false;
};

USTRUCT(BlueprintType)
struct FCPActiveEffectInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag EffectTag;

	UPROPERTY(BlueprintReadOnly)
	int64 AppliedAtWorldMinute = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 ExpiresAtWorldMinute = 0;

	UPROPERTY(BlueprintReadOnly)
	float Magnitude = 1.0f;
};

USTRUCT(BlueprintType)
struct FCPNPCEffectSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, FCPActiveEffectInfo> ActiveEffects;
};

USTRUCT()
struct CREATEPOTION_API FCPNPCSaveData
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<FName, FCPNPCEffectSaveData> NPCEffects;
	
	UPROPERTY()
	TMap<FName, FCPNPCMovementSaveData> NPCMovements;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnNPCEffectApplied, FName, NPCId, FGameplayTag, EffectTag, int64, ExpiresAtWorldMinute, float, Magnitude);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCEffectExpired, FName, NPCId, FGameplayTag, EffectTag);

UCLASS()
class CREATEPOTION_API UCPNPCSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	FCPNPCSaveData GetSaveData();
	void LoadSaveData(const FCPNPCSaveData& SaveData);

	UFUNCTION(BlueprintCallable, Category = "NPC|Potion")
	void RegisterNPCEffect(FName NPCId, FGameplayTag EffectTag, int64 DurationWorldMinutes, float Magnitude = 1.0f);

	UFUNCTION(BlueprintPure, Category = "NPC|Potion")
	bool GetNPCEffectData(FName NPCId, FCPNPCEffectSaveData& OutSaveData) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Potion")
	void ClearNPCEffect(FName NPCId, FGameplayTag EffectTag);

	void RegisterActiveNPC(FName NPCId, ACPBaseNPC* NPC);
	void UnregisterActiveNPC(FName NPCId, ACPBaseNPC* NPC);

	void SaveNPCMovementData(FName NPCId, const FCPNPCMovementSaveData& MovementData);
	bool GetNPCMovementData(FName NPCId, FCPNPCMovementSaveData& OutMovementData) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetNPCActiveStateById(FName NPCId, bool bActive, ACPBaseNPC* IgnoreNPC = nullptr);

	UPROPERTY(BlueprintAssignable, Category = "NPC|Potion")
	FOnNPCEffectApplied OnNPCEffectApplied;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Potion")
	FOnNPCEffectExpired OnNPCEffectExpired;

protected:
	UFUNCTION()
	void OnWorldClockMinuteTick(int64 CurrentWorldMinute);

private:
	void NotifyEffectApplied(FName NPCId, FGameplayTag EffectTag, int64 ExpiresAtWorldMinute, float Magnitude);
	void NotifyEffectExpired(FName NPCId, FGameplayTag EffectTag);

	UPROPERTY()
	TMap<FName, FCPNPCEffectSaveData> NPCStateRepository;
	
	TMap<FName, TArray<TWeakObjectPtr<ACPBaseNPC>>> ActiveNPCRegistry;
	int64 CachedCurrentWorldMinute = 0;

	UPROPERTY()
	TMap<FName, FCPNPCMovementSaveData> NPCMovementRepository;
};