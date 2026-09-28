// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/CPTagDefinitionTypes.h"
#include "Engine/DataTable.h"

const FCPTagDefinitionRow* FCPTagDefinitionLookup::FindRow(const UDataTable* Table, const FGameplayTag& Tag)
{
	if (!Table)
	{
		return nullptr;
	}

	for (const auto& Pair : Table->GetRowMap())
	{
		const FCPTagDefinitionRow* Row = reinterpret_cast<const FCPTagDefinitionRow*>(Pair.Value);
		if (Row && Row->Tag == Tag)
		{
			return Row;
		}
	}
	return nullptr;
}
