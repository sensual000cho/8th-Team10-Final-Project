#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "CPNPCSubsystem.generated.h"

class ACPBaseNPC;

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnNPCEffectApplied, FName, NPCId, FGameplayTag, EffectTag, int64, ExpiresAtWorldMinute, float, Magnitude);
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

	void RegisterActiveNPC(FName NPCId, ACPBaseNPC* NPC);
	void UnregisterActiveNPC(FName NPCId, ACPBaseNPC* NPC);

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
	TMap<FName, TWeakObjectPtr<ACPBaseNPC>> ActiveNPCRegistry;
	int64 CachedCurrentWorldMinute = 0;
};