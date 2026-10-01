// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance/Subsystem/CPCodexSubsystem.h"

#include "Data/CPForageableItemData.h"
#include "Data/NPC/CPNPCDataAsset.h"

FCPCodexSaveData UCPCodexSubsystem::GetSaveData() const
{
	FCPCodexSaveData SaveData;
	
	ForageableCodexEntries.GenerateValueArray(SaveData.ForageableEntries);
	SaveData.DiscoveredTags = DiscoveredTags;
	NPCCodexEntries.GenerateValueArray(SaveData.NPCEntries);
	
	return SaveData;
}

void UCPCodexSubsystem::LoadSaveData(const FCPCodexSaveData& SaveData)
{
	ForageableCodexEntries.Empty(SaveData.ForageableEntries.Num());
	// 채집물
	for (const FCPForageableCodexEntry& Entry : SaveData.ForageableEntries) {
		ForageableCodexEntries.Add(Entry.Entry, Entry);
	}
	
	// 발견한 태그
	DiscoveredTags = SaveData.DiscoveredTags;
	
	// NPC
	NPCCodexEntries.Empty(SaveData.NPCEntries.Num());
	for (const FCPNPCCodexEntry& Entry : SaveData.NPCEntries) {
		NPCCodexEntries.Add(Entry.Entry, Entry);
	}
}

void UCPCodexSubsystem::ResetData()
{
	ForageableCodexEntries.Reset();
	DiscoveredTags.Reset();
	NPCCodexEntries.Reset();
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

UCPForageableItemData* UCPCodexSubsystem::FindRegisteredItemByTag(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid()) return nullptr;
	
	for (const TPair<TObjectPtr<UCPForageableItemData>, FCPForageableCodexEntry>& Pair : ForageableCodexEntries){
		UCPForageableItemData* ItemData = Pair.Key.Get();
		if (!ItemData || ItemData->TagAxes[0] != Tag) continue;
		return ItemData;
	}
	return nullptr;
}

bool UCPCodexSubsystem::DiscoverTag(FGameplayTag Tag)
{
	if (!Tag.IsValid()) return false;
	if (const bool* TagState = DiscoveredTags.Find(Tag)){
		if (*TagState) return false;
	}
	
	DiscoveredTags.Add(Tag, true);
	OnForageableCodexUpdated.Broadcast();
	return true;
}

bool UCPCodexSubsystem::IsTagDiscovered(FGameplayTag Tag) const
{
	const bool* TagState = DiscoveredTags.Find(Tag);
	return TagState && *TagState;
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

TArray<UCPNPCDataAsset*> UCPCodexSubsystem::GetRegisteredNPCDatas() const
{
	TArray<TObjectPtr<UCPNPCDataAsset>> Keys;
	NPCCodexEntries.GenerateKeyArray(Keys);
	
	TArray<UCPNPCDataAsset*> NPCDatas;
	NPCDatas.Reserve(Keys.Num());
	
	for (const TObjectPtr<UCPNPCDataAsset>& Key : Keys) {
		NPCDatas.Add(Key.Get());
	}
	
	NPCDatas.Sort([](const UCPNPCDataAsset& A, const UCPNPCDataAsset& B)
	{
		return A.NPCName.ToString() < B.NPCName.ToString();
	});
	
	return NPCDatas;
}

bool UCPCodexSubsystem::RecordForageableEntry(UCPForageableItemData* ItemData)
{
	if (!ItemData) return false;
	
	DiscoveredTags.Add(ItemData->TagAxes[0], true);
	
	// 등록이 되어 있는 경우 채집 횟수 증가
	if (FCPForageableCodexEntry* CodexEntry = ForageableCodexEntries.Find(ItemData)){
		CodexEntry->HarvestCount++;
		
		if (CodexEntry->Level < MaxLevel && CodexEntry->HarvestCount >= RequiredCount[CodexEntry->Level]){
			CodexEntry->Level++;
		}
		
		OnForageableCodexUpdated.Broadcast();
		return true;
	}
	else
	{
		// 채집물 등록
		FCPForageableCodexEntry NewEntry;
		NewEntry.Entry = ItemData;
		ForageableCodexEntries.Add(ItemData, NewEntry);
		
		OnForageableCodexUpdated.Broadcast();
		OnForageableCodexRegistered.Broadcast(ItemData);
		return true;	
	}
	
	
}

bool UCPCodexSubsystem::RegisterNPCEntry(UCPNPCDataAsset* NPCData)
{
	if (NPCCodexEntries.Contains(NPCData)) return true;
	
	FCPNPCCodexEntry NewEntry;
	NewEntry.Entry = NPCData;
	NPCCodexEntries.Add(NPCData, NewEntry);
	
	OnNPCCodexUpdated.Broadcast();
	return true;
}
