// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CPCodexSubsystem.generated.h"

class UCPForageableItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForageableCodexUpdated);

USTRUCT(BlueprintType)
struct FCPForageableCodexEntry
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	TObjectPtr<UCPForageableItemData> Entry = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Codex")
	int32 Level = 1;
};

UCLASS()
class CREATEPOTION_API UCPCodexSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 등록된 전체 채집물을 반환
	UFUNCTION(BlueprintPure, Category = "Codex|Forageable")
	TArray<FCPForageableCodexEntry> GetForageableCodexEntries() const;
	
	// 도감 SubSystem에 새로운 채집물을 등록
	UFUNCTION(BlueprintCallable, Category = "Codex|Forageable")
	bool RegisterForageableEntry(UCPForageableItemData* ItemData);
	
	// 등록된 채집물의 정보 레벨을 증가(~4)
	UFUNCTION(BlueprintCallable, Category = "Codex|Forageable")
	bool IncreaseForageableLevel(UCPForageableItemData* ItemData);
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Codex|Forageable")
	FOnForageableCodexUpdated OnForageableCodexUpdated;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Codex|Forageable")
	TArray<FCPForageableCodexEntry> ForageableCodexEntries;
};
