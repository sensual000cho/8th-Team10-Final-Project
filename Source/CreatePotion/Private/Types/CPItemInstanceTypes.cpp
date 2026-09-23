// CPItemInstanceTypes.cpp

#include "Types/CPItemInstanceTypes.h"
#include "Settings/CPPotionVisualSettings.h"
#include "Data/CPTagDefinitionTypes.h"
#include "Data/CPForageableItemData.h"
#include "Settings/CPDTSettings.h"
#include "Engine/DataTable.h"

FLinearColor FCPItemInstance::GetLiquidColor() const
{
	const UCPPotionVisualSettings* Settings = GetDefault<UCPPotionVisualSettings>();

	if (CurrentEffects.Num() == 0)
	{
		return Settings->DefaultLiquidColor;
	}

	FLinearColor Blended = FLinearColor::Black;
	int32 MatchedCount = 0;

	for (const FGameplayTag& Tag : CurrentEffects)
	{
		if (const FLinearColor* Found = Settings->EffectColors.Find(Tag))
		{
			Blended += *Found;
			MatchedCount++;
		}
	}

	if (MatchedCount == 0)
	{
		return Settings->DefaultLiquidColor;
	}

	return Blended / static_cast<float>(MatchedCount); // 여러 효과면 색을 평균 혼합
}

FText FCPItemInstance::GetDisplayName() const
{
	if (!SourceItemData)
	{
		return FText::GetEmpty();
	}

	// 효과가 없는 일반 재료는 원본 DisplayName 그대로
	if (CurrentEffects.Num() == 0)
	{
		return SourceItemData->DisplayName;
	}

	const UCPDTSettings* Settings = GetDefault<UCPDTSettings>();
	UDataTable* Table = Settings->TagDefinitionTable.LoadSynchronous();
	if (!Table)
	{
		return SourceItemData->DisplayName;
	}

	// 효과 태그 순서대로 DisplayName(수식어)을 이어 붙임
	FString CombinedPrefix;
	for (const FGameplayTag& Tag : CurrentEffects)
	{
		if (const FCPTagDefinitionRow* Row = FCPTagDefinitionLookup::FindRow(Table, Tag))
		{
			if (!Row->DisplayName.IsEmpty())
			{
				if (!CombinedPrefix.IsEmpty())
				{
					CombinedPrefix += TEXT(" ");
				}
				CombinedPrefix += Row->DisplayName.ToString();
			}
		}
	}

	if (CombinedPrefix.IsEmpty())
	{
		return SourceItemData->DisplayName; // 매핑된 태그가 하나도 없으면 원본 이름
	}

	return FText::FromString(CombinedPrefix + TEXT(" ") + SourceItemData->DisplayName.ToString());
}
