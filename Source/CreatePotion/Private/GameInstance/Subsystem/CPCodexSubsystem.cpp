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

bool UCPCodexSubsystem::IsForageableRegistered(UCPForageableItemData* ItemData) const
{
	if (!ItemData) return false;
	
	return ForageableCodexEntries.Contains(ItemData);
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
