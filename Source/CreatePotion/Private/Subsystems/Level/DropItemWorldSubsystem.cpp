// DropItemWorldSubsystem.cpp

#include "Subsystems/Level/DropItemWorldSubsystem.h"
#include "CreatePotion.h"   // 로그용 헤더

#include "Components/CPItemContainerComponent.h"
#include "Lab/Actor/CPAlchemyProp.h"

void UDropItemWorldSubsystem::RegisterDroppedProp(ACPAlchemyProp* Prop)
{
	if (Prop)
	{
		ActiveDroppedProps.AddUnique(Prop);
	}
}

void UDropItemWorldSubsystem::UnregisterDroppedProp(ACPAlchemyProp* Prop)
{
	ActiveDroppedProps.Remove(Prop);
}

void UDropItemWorldSubsystem::CollectAllDroppedItems(UCPItemContainerComponent* TargetContainer)
{
	if (!TargetContainer)
	{
		return;
	}

	// 목록에서 즉시 제거해도 인덱스가 안 꼬이도록 뒤에서부터 순회
	for (int32 i = ActiveDroppedProps.Num() - 1; i >= 0; --i)
	{
		ACPAlchemyProp* Prop = ActiveDroppedProps[i];

		// 등록 누락이나 다른 경로로 이미 파괴된 액터에 대한 확인
		if (!IsValid(Prop))
		{
			ActiveDroppedProps.RemoveAt(i);
			continue;
		}

		const FCPItemInstance& Ingredient = Prop->GetWorkingIngredient();
		if (!Ingredient.IsValid())
		{
			continue; // 유효하지 않은 데이터면 건드리지 않고 넘어감
		}

		int32 Leftover = TargetContainer->TryGetItemFromData(Ingredient.SourceItemData, 1);

		if (Leftover <= 0)
		{
			// 완전히 수거 성공 -> Prop 제거, 목록에서도 제거
			ActiveDroppedProps.RemoveAt(i);
			Prop->Destroy();
		}
		else
		{
			// TODO[Container] : 인벤토리 공간 부족 -> Prop을 그대로 남겨둠 (목록에도 유지)
			UE_LOG(LogContainer, Warning, TEXT("[Collect] 인벤토리 공간 부족으로 [%s] 수거 실패, 바닥에 유지"),
				*Ingredient.SourceItemData->DisplayName.ToString());
		}
	}
}
