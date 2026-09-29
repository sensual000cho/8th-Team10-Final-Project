// CPItemWheelWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPItemWheelWidget.generated.h"

class UImage;
class UCanvasPanel;
class UCPInventoryComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFocusedPotionChangedSignature, int32 /*GridIndex, 없으면 -1*/);

UCLASS()
class CREATEPOTION_API UCPItemWheelWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()

public:
	// PC가 인벤토리를 확보한 시점에 한 번 호출
	void BindInventory(UCPInventoryComponent* InInventory);

	// 나중에 "휠키로 장착" 로직에서 쓸 조회 함수
	UFUNCTION(BlueprintPure, Category = "ItemWheel")
	bool GetFocusedPotionGridIndex(int32& OutGridIndex) const;

	UFUNCTION(BlueprintCallable, Category = "ItemWheel")
	void HandleScrollInput(float ScrollDelta);

	bool TryScrollAndPickNextFocus(float ScrollDelta, int32& OutGridIndex);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void OnInventoryUpdated();

	void RefreshPotionList();

	void RefreshIcons();

	void ShowWheelTemporarily();

	UFUNCTION()
	void HideWheel();

	void SetSlotVisible(UImage* IconWidget, UImage* LiquidWidget, bool bVisible);

private:
	UPROPERTY()
	UCPInventoryComponent* BoundInventory;

	// 필터링된 "포션인 항목"들의 ContainerItems 배열 내 인덱스 목록
	UPROPERTY()
	TArray<int32> PotionArrayIndices;

	// PotionArrayIndices 안에서 지금 포커스 중인 위치
	int32 FocusedListIndex = 0;

	FTimerHandle HideTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "ItemWheel")
	float AutoHideDelay = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "ItemWheel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NearIconAlpha = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "ItemWheel", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FarIconAlpha = 0.2f;

	UPROPERTY(meta = (BindWidgetOptional))
	UCanvasPanel* RootCanvas;

	UPROPERTY(meta = (BindWidget))
	UImage* Left2Icon;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Left2Liquid;

	UPROPERTY(meta = (BindWidget))
	UImage* Left1Icon;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Left1Liquid;

	UPROPERTY(meta = (BindWidget))
	UImage* FocusIcon;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* FocusLiquid;

	UPROPERTY(meta = (BindWidget))
	UImage* Right1Icon;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Right1Liquid;

	UPROPERTY(meta = (BindWidget))
	UImage* Right2Icon;
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Right2Liquid;

#pragma region SlotHighlight
public:
	void BroadcastFocusedPotion();

public:
	FOnFocusedPotionChangedSignature OnFocusedPotionChanged;

#pragma endregion

#pragma region Sliding
private:
	void UpdateSlotScales(float Progress);

	float GetTargetScaleForOffset(int32 Offset) const;

protected:
	UPROPERTY(meta = (BindWidget))
	UPanelWidget* IconRow;

private:
	UPROPERTY(EditDefaultsOnly, Category = "ItemWheel|Animation")
	float FocusScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "ItemWheel|Animation")
	float NearScale = 0.33f; // Left1/Right1

	UPROPERTY(EditDefaultsOnly, Category = "ItemWheel|Animation")
	float FarScale = 0.25f;  // Left2/Right2

	UPROPERTY(EditDefaultsOnly, Category = "ItemWheel|Animation")
	float SlideSpeed = 5.f;

	// 아이콘 한 칸의 실제 폭(패딩 포함), IconRow 안 슬롯 간격과 일치시켜야 함
	UPROPERTY(EditDefaultsOnly, Category = "ItemWheel|Animation")
	float SlotStride = 90.f; 

	int32 PendingDirection = 0;

	float SlideProgress = 0.f;
#pragma endregion
};
