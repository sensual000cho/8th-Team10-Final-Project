#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "CPNPCSubsystem.generated.h"

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

	// GA가 넘긴 효과 세기(예: Giant 스케일 배율).
	UPROPERTY(BlueprintReadOnly)
	float Magnitude = 1.0f;
};

// 하나의 NPCId(=NPCData DA)가 현재 갖고 있는 모든 지속 효과 목록.
USTRUCT(BlueprintType)
struct FCPNPCEffectSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, FCPActiveEffectInfo> ActiveEffects;
};

//  NPC에게 새로운 효과가 적용될 때
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnNPCEffectApplied, FName, NPCId, FGameplayTag, EffectTag, int64, ExpiresAtWorldMinute, float, Magnitude);

// NPC의 효과 지속시간이 만료되거나 해제될 때
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCEffectExpired, FName, NPCId, FGameplayTag, EffectTag);

UCLASS()
class CREATEPOTION_API UCPNPCSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "NPC|Potion")
	void RegisterNPCEffect(FName NPCId, FGameplayTag EffectTag, int64 DurationWorldMinutes, float Magnitude = 1.0f);

	UFUNCTION(BlueprintPure, Category = "NPC|Potion")
	bool GetNPCEffectData(FName NPCId, FCPNPCEffectSaveData& OutSaveData) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Potion")
	void ClearNPCEffect(FName NPCId, FGameplayTag EffectTag);

	// 디버그용
	UFUNCTION(BlueprintCallable, Category = "NPC|Potion|Debug")
	void DebugAddNPCEffect(FName NPCId, FGameplayTag EffectTag, int64 DurationWorldMinutes);

	UPROPERTY(BlueprintAssignable, Category = "NPC|Potion")
	FOnNPCEffectApplied OnNPCEffectApplied;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Potion")
	FOnNPCEffectExpired OnNPCEffectExpired;

protected:
	UFUNCTION()
	void OnWorldClockMinuteTick(int64 CurrentWorldMinute);

private:
	UPROPERTY()
	TMap<FName, FCPNPCEffectSaveData> NPCStateRepository;

	int64 CachedCurrentWorldMinute = 0;
};