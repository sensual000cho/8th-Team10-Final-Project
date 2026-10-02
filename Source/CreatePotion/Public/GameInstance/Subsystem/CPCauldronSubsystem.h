// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/CPItemInstanceTypes.h"
#include "CPCauldronSubsystem.generated.h"

class UCPCauldronComponent;

USTRUCT()
struct CREATEPOTION_API FCPCauldronSaveData
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FCPItemInstance> IngredientInstances;
	
	UPROPERTY()
	TArray<FGameplayTag> IngredientTags;
	
	UPROPERTY()
	TArray<FGameplayTag> CombinedTags;
};

UCLASS()
class CREATEPOTION_API UCPCauldronSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 가마솥 컴포넌트 등록
	void RegisterCauldron(UCPCauldronComponent* InCauldronComponent);
	void UnRegisterCauldron(UCPCauldronComponent* InCauldronComponent);
	
	// 게임 세이브/로드
	FCPCauldronSaveData GetSaveData();
	void LoadSaveData(const FCPCauldronSaveData& InSaveData);
	void ResetData();
	
private:
	UPROPERTY()
	FCPCauldronSaveData CauldronData;
	
	TWeakObjectPtr<UCPCauldronComponent> ActiveCauldronComponent;
	
	bool bHasCauldronData = false;
};
