// CPTagDefinitionTypes.cpp

#include "Data/CPTagDefinitionTypes.h"
#include "Settings/CPDTSettings.h"
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

FText FCPTagDefinitionLookup::GetDisplayName(const FGameplayTag& Tag)
{
	const UCPDTSettings* Settings = GetDefault<UCPDTSettings>();
	const UDataTable* Table = Settings ? Settings->TagDefinitionTable.LoadSynchronous() : nullptr;

	if (const FCPTagDefinitionRow* Row = FindRow(Table, Tag))
	{
		if (!Row->DisplayName.IsEmpty())
		{
			return Row->DisplayName;
		}
	}

	// 테이블에 없거나 표시 이름이 비어 있으면 태그 자체 이름으로 대체
	return FText::FromName(Tag.GetTagName());
}
