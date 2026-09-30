// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPCodexSubsystem.generated.h"

class UCPNPCDataAsset;
class UCPForageableItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForageableCodexUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNPCCodexUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnForageableCodexRegistered, UCPForageableItemData*, ItemData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnForageableCodexLevelUp, UCPForageableItemData*, ItemData, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNPCCodexRegistered, UCPNPCDataAsset*, NPCData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCodexItemSelected, UCPForageableItemData*, ItemData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCodexNPCSelected, UCPNPCDataAsset*, NPCData);


USTRUCT(BlueprintType)
struct FCPForageableCodexEntry
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPForageableItemData> Entry = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	int32 Level = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	int32 HarvestCount = 1;
};

USTRUCT(BlueprintType)
struct FCPNPCCodexEntry
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPNPCDataAsset> Entry = nullptr;
};

USTRUCT(BlueprintType)
struct FCPCodexSaveData
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<FCPForageableCodexEntry> ForageableEntries;
	
	UPROPERTY()
	TMap<FGameplayTag, bool> DiscoveredTags;

	UPROPERTY()
	TArray<FCPNPCCodexEntry> NPCEntries;
};

UCLASS()
class CREATEPOTION_API UCPCodexSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// Game Save
	FCPCodexSaveData GetSaveData() const;
	void LoadSaveData(const FCPCodexSaveData& SaveData);
	
	// 등록된 전체 채집물 DA를 반환
	UFUNCTION(BlueprintPure, Category = "Codex|Forageable")
	TArray<UCPForageableItemData*> GetRegisteredForageableItems() const;
	
	UFUNCTION(BlueprintPure, Category = "Codex|Tag")
	UCPForageableItemData* FindRegisteredItemByTag(const FGameplayTag& Tag) const;
	
	UFUNCTION(BlueprintCallable, Category = "Codex|Tag")
	bool DiscoverTag(FGameplayTag Tag);
	
	UFUNCTION(BlueprintPure, Category = "Codex|Tag")
	bool IsTagDiscovered(FGameplayTag Tag) const;
	
	// 등록된 채집물이 있는 경우 정보를 반환, 등록 여부만 필요할 경우 ItemData만 인자로 전달
	bool GetForageableEntry(UCPForageableItemData* ItemData, FCPForageableCodexEntry* OutForageableEntry = nullptr) const;
	
	UFUNCTION(BlueprintPure, Category = "Codex|NPC")
	TArray<UCPNPCDataAsset*> GetRegisteredNPCDatas() const;
	
	// 도감 SubSystem에 새로운 채집물을 등록 및 채집 횟수 증가
	UFUNCTION(BlueprintCallable, Category = "Codex|Forageable")
	bool RecordForageableEntry(UCPForageableItemData* ItemData);
	
	UFUNCTION(BlueprintCallable, Category = "Codex|NPC")
	bool RegisterNPCEntry(UCPNPCDataAsset* NPCData);
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Codex|Forageable")
	FOnForageableCodexUpdated OnForageableCodexUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Codex|Forageable")
	FOnCodexItemSelected OnCodexItemSelected;
	
	UPROPERTY(BlueprintAssignable, Category = "Codex|NPC")
	FOnNPCCodexUpdated OnNPCCodexUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Codex|NPC")
	FOnCodexNPCSelected OnCodexNPCSelected;

	UPROPERTY(BlueprintAssignable, Category = "Codex|Forageable")
	FOnForageableCodexRegistered OnForageableCodexRegistered;
	
	UPROPERTY(BlueprintAssignable, Category = "Codex|Forageable")
	FOnForageableCodexLevelUp OnForageableCodexLevelUp;
	
	UPROPERTY(BlueprintAssignable, Category = "Codex|Forageable")
	FOnNPCCodexRegistered OnNPCCodexRegistered;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Codex|Forageable")
	TMap<TObjectPtr<UCPForageableItemData>, FCPForageableCodexEntry> ForageableCodexEntries;
	
	UPROPERTY(BlueprintReadOnly, Category = "Codex|Tag")
	TMap<FGameplayTag, bool> DiscoveredTags;
	
	UPROPERTY(BlueprintReadOnly, Category = "Codex|NPC")
	TMap<TObjectPtr<UCPNPCDataAsset>, FCPNPCCodexEntry> NPCCodexEntries;
	
private:
	inline static constexpr int32 RequiredCount[] = {1, 3, 6, 10};
	static constexpr int32 MaxLevel = 4;
};
