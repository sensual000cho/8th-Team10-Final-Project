// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance/Subsystem/CPCodexSubsystem.h"

#include "Data/CPForageableItemData.h"

TArray<FCPForageableCodexEntry> UCPCodexSubsystem::GetForageableCodexEntries() const
{
	return ForageableCodexEntries;
}

bool UCPCodexSubsystem::RegisterForageableEntry(UCPForageableItemData* ItemData)
{
	if (!ItemData) return false;
	
	for (FCPForageableCodexEntry& CodexEntry : ForageableCodexEntries){
		if (CodexEntry.Entry == ItemData) return false;
	}
	
	FCPForageableCodexEntry NewEntry;
	NewEntry.Entry = ItemData;
	ForageableCodexEntries.Add(NewEntry);
	
	ForageableCodexEntries.Sort([](const FCPForageableCodexEntry& A, const FCPForageableCodexEntry& B)
	{
		const FString AName = A.Entry->DisplayName.ToString();
		const FString BName = B.Entry->DisplayName.ToString();
		
		return AName < BName;
	});
	
	return true;
}

bool UCPCodexSubsystem::IncreasedForageableLevel(UCPForageableItemData* ItemData)
{
	if (!ItemData) return false;
	
	for (FCPForageableCodexEntry& CodexEntry : ForageableCodexEntries){
		if (CodexEntry.Entry == ItemData){
			if (CodexEntry.Level >= 4) return false;
			CodexEntry.Level++;
			return true;
		}
	}
	
	return false;
}
