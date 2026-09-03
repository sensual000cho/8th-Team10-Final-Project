// CPItemWheelWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPItemWheelWidget.generated.h"

class UImage;
class UCPInventoryComponent;

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

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnInventoryUpdated();

	void RefreshPotionList();

	void RefreshIcons();

	void ShowWheelTemporarily();

	UFUNCTION()
	void HideWheel();

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

	UPROPERTY(meta = (BindWidget))
	UImage* PrevIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* FocusIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* NextIcon;

};
