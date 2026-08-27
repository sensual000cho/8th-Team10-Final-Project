// CPContainerGridWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPContainerGridWidget.generated.h"

class UUniformGridPanel;
class UCPBlankGridSlotWidget;
class UCPItemSlotWidget;
class UCPItemContainerComponent;
class UCanvasPanel;
struct FContainerItem;

UCLASS()
class CREATEPOTION_API UCPContainerGridWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()
public:
	// 컴포넌트의 설정(Slot1D, Grid2D)에 맞춰 칸(Slot)들을 최초 1회 생성
	UFUNCTION(BlueprintCallable, Category = "Container|Grid")
	void InitializeGrid(UCPItemContainerComponent* TargetContainer);

	// 아이템 배열 데이터를 받아 슬롯들의 화면을 최신화
	UFUNCTION(BlueprintCallable, Category = "Container|Grid")
	void UpdateGrid(const TArray<FContainerItem>& ContainerItems);
	
protected:
	// 한 칸(Slot)당 생성할 슬롯 위젯
	UPROPERTY(EditDefaultsOnly, Category = "Container|UI")
	TSubclassOf<UCPBlankGridSlotWidget> BackgroundSlotClass;

	// 캔버스 위에 올라갈 실제 아이템 위젯
	UPROPERTY(EditDefaultsOnly, Category = "Container|UI")
	TSubclassOf<UCPItemSlotWidget> ItemWidgetClass;

	// 추후 다른 클래스와의 상호작용을 위해 캐싱
	UPROPERTY()
	UCPItemContainerComponent* CachedContainer;

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* ItemGrid;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* ItemCanvas;
};
