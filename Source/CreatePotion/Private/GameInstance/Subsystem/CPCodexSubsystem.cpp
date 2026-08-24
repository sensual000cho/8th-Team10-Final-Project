// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance/Subsystem/CPCodexSubsystem.h"

#include "Data/CPForageableItemData.h"

TArray<FCPForageableCodexEntry> UCPCodexSubsystem::GetForageableCodexEntries() const
{
	TArray<FCPForageableCodexEntry> CodexEntries;
	ForageableCodexEntries.GenerateValueArray(CodexEntries);
	
	// 사전 순 정렬
	CodexEntries.Sort([](const FCPForageableCodexEntry& A, const FCPForageableCodexEntry& B)
	{
		const FString AName = A.Entry->DisplayName.ToString();
		const FString BName = B.Entry->DisplayName.ToString();
		
		return AName < BName;
	});
	
	return CodexEntries;
}

TArray<UCPForageableItemData*> UCPCodexSubsystem::GetRegisteredForageableItems() const
{
	TArray<TObjectPtr<UCPForageableItemData>> Keys;
	ForageableCodexEntries.GenerateKeyArray(Keys);
	
	TArray<UCPForageableItemData*> Items;
	Items.Reserve(Keys.Num());
	
	for (const TObjectPtr<UCPForageableItemData>& Key : Keys) {
		Items.Add(Key.Get());
	}
	
	// 사전 순 정렬
	Items.Sort([](const UCPForageableItemData& A, const UCPForageableItemData& B)
	{
		return A.DisplayName.ToString() < B.DisplayName.ToString();
	});
	
	return Items;
}

bool UCPCodexSubsystem::GetForageableEntry(UCPForageableItemData* ItemData, 
	FCPForageableCodexEntry* OutForageableEntry) const
{
	if (!ItemData) return false;
	
	const FCPForageableCodexEntry* CodexEntry = ForageableCodexEntries.Find(ItemData);
	if (!CodexEntry) return false;
	
	// 구조체 인자가 들어왔을 경우 Subsystem의 값으로 초기화
	if (OutForageableEntry){
		*OutForageableEntry = *CodexEntry;	
	}
	return true;
}

bool UCPCodexSubsystem::RecordForageableEntry(UCPForageableItemData* ItemData)
{
	if (!ItemData) return false;
	
	// 등록이 되어 있는 경우 채집 횟수 증가
	if (FCPForageableCodexEntry* CodexEntry = ForageableCodexEntries.Find(ItemData)){
		CodexEntry->HarvestCount++;
		OnForageableCodexUpdated.Broadcast();
		return true;
	}
	
	// 채집물 등록
	FCPForageableCodexEntry NewEntry;
	NewEntry.Entry = ItemData;
	ForageableCodexEntries.Add(ItemData, NewEntry);
	
	OnForageableCodexUpdated.Broadcast();
	return true;
}

bool UCPCodexSubsystem::IncreaseForageableLevel(UCPForageableItemData* ItemData)
{
	if (!ItemData) return false;
	
	FCPForageableCodexEntry* CodexEntry = ForageableCodexEntries.Find(ItemData);
	if (!CodexEntry || CodexEntry->Level >= 4) return false;

	CodexEntry->Level++;
	OnForageableCodexUpdated.Broadcast();
	return true;
}
