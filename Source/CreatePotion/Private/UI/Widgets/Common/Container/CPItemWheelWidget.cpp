// CPItemWheelWidget.cpp

#include "UI/Widgets/Common/Container/CPItemWheelWidget.h"
#include "Components/Image.h"

#include "CreatePotion.h"
#include "Components/CPInventoryComponent.h"
#include "Data/CPForageableItemData.h"

void UCPItemWheelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 루트는 항상 Visible로 유지 (휠 입력을 계속 받아야 하므로)
	SetVisibility(ESlateVisibility::Visible);

	// 평소엔 아이콘만 안 보이게
	if (PrevIcon)
	{
		PrevIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (FocusIcon)
	{
		FocusIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (NextIcon)
	{
		NextIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCPItemWheelWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
	Super::NativeDestruct();
}

void UCPItemWheelWidget::BindInventory(UCPInventoryComponent* InInventory)
{
	if (!InInventory || BoundInventory == InInventory)
	{
		return;
	}

	BoundInventory = InInventory;
	BoundInventory->OnContainerUpdated.AddDynamic(this, &UCPItemWheelWidget::OnInventoryUpdated);

	RefreshPotionList();
}

void UCPItemWheelWidget::OnInventoryUpdated()
{
	RefreshPotionList();

	// 지금 아이콘이 보이는 중(=휠 조작 중)이었다면 최신 목록으로 다시 그려줌
	if (FocusIcon && FocusIcon->GetVisibility() != ESlateVisibility::Collapsed)
	{
		RefreshIcons();
	}
}

void UCPItemWheelWidget::RefreshPotionList()
{
	PotionArrayIndices.Empty();

	if (!BoundInventory)
	{
		return;
	}

	for (int32 i = 0; i < BoundInventory->ContainerItems.Num(); ++i)
	{
		const FContainerItem& Item = BoundInventory->ContainerItems[i];
		if (Item.Instance.SourceItemData && Item.Instance.CurrentEffects.Num() > 0)
		{
			PotionArrayIndices.Add(i);
		}
	}

	// 목록이 줄어들어 포커스 인덱스가 범위를 벗어난 경우 보정
	if (PotionArrayIndices.Num() > 0)
	{
		FocusedListIndex = FMath::Clamp(FocusedListIndex, 0, PotionArrayIndices.Num() - 1);
	}
	else
	{
		FocusedListIndex = 0;
	}

	UE_LOG(LogContainer, Warning, TEXT("[ItemWheel] 포션 목록 갱신, 개수: %d"), PotionArrayIndices.Num());
}

void UCPItemWheelWidget::RefreshIcons()
{
	UE_LOG(LogContainer, Warning, TEXT("[ItemWheel] RefreshIcons 호출, PotionCount=%d, FocusedIdx=%d"),
		PotionArrayIndices.Num(), FocusedListIndex);

	if (!BoundInventory || PotionArrayIndices.Num() == 0)
	{
		if (PrevIcon)
		{
			PrevIcon->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (FocusIcon)
		{
			FocusIcon->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (NextIcon)
		{
			NextIcon->SetVisibility(ESlateVisibility::Collapsed);
		}

		return;
	}

	int32 Count = PotionArrayIndices.Num();

	auto SetIconFromArrayIndex = [this](UImage* IconWidget, int32 ContainerArrayIdx, float Alpha)
		{
			if (!IconWidget) return;

			const FContainerItem& Item = BoundInventory->ContainerItems[ContainerArrayIdx];
			UTexture2D* LoadedTexture = Item.Instance.SourceItemData->Icon.LoadSynchronous();
			if (LoadedTexture)
			{
				IconWidget->SetBrushFromTexture(LoadedTexture);
			}
			IconWidget->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Alpha));
			IconWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		};

	// 포커스 아이콘 (항상 존재 - 목록이 비어있지 않은 이상)
	SetIconFromArrayIndex(FocusIcon, PotionArrayIndices[FocusedListIndex], 1.0f);

	// 이전/다음 - 포션이 1개뿐이면 이웃이 자기 자신과 같아지므로 숨김 처리
	if (Count > 1)
	{
		int32 PrevListIdx = (FocusedListIndex - 1 + Count) % Count;
		int32 NextListIdx = (FocusedListIndex + 1) % Count;

		SetIconFromArrayIndex(PrevIcon, PotionArrayIndices[PrevListIdx], NearIconAlpha);
		SetIconFromArrayIndex(NextIcon, PotionArrayIndices[NextListIdx], NearIconAlpha);
	}
	else
	{
		if (PrevIcon)
		{
			PrevIcon->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (NextIcon)
		{
			NextIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UCPItemWheelWidget::ShowWheelTemporarily()
{
	GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		HideTimerHandle, 
		this, 
		&UCPItemWheelWidget::HideWheel, 
		AutoHideDelay, 
		false
	);
}

void UCPItemWheelWidget::HideWheel()
{
	if (PrevIcon)
	{
		PrevIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (FocusIcon)
	{
		FocusIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (NextIcon)
	{
		NextIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UCPItemWheelWidget::GetFocusedPotionGridIndex(int32& OutGridIndex) const
{
	if (!BoundInventory || PotionArrayIndices.Num() == 0)
	{
		return false;
	}

	int32 ArrayIdx = PotionArrayIndices[FocusedListIndex];
	OutGridIndex = BoundInventory->ContainerItems[ArrayIdx].GridIndex;
	return true;
}

void UCPItemWheelWidget::HandleScrollInput(float ScrollDelta)
{
	if (PotionArrayIndices.Num() == 0)
	{
		return; // 목록이 비어있으면 할 일 없음
	}

	int32 Direction = (ScrollDelta > 0.f) ? -1 : 1;

	int32 Count = PotionArrayIndices.Num();
	FocusedListIndex = (FocusedListIndex + Direction + Count) % Count;

	RefreshIcons();
	ShowWheelTemporarily();
}
