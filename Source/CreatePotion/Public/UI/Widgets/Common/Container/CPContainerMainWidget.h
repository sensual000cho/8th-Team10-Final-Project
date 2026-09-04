// CPContainerMainWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPContainerMainWidget.generated.h"

class UCPContainerGridWidget;
class UCPItemContainerComponent;
class UBorder;
class UCPMoneyWidget;
class UButton;

UCLASS()
class CREATEPOTION_API UCPContainerMainWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()

public:
	// 창이 처음 초기화될 때 컴포넌트와 연결해주는 함수
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void BindContainer(UCPItemContainerComponent* InContainer);

	// 아이템 획득/사용 시 UI를 새로고침하는 함수
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void UpdateUI();

protected:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void HandleCloseRequested();

private:
	UFUNCTION()
	void OnCloseButtonClicked();

protected:
	// 실제 데이터를 가져올 컨테이너 컴포넌트
	UPROPERTY(BlueprintReadOnly, Category = "Container")
	UCPItemContainerComponent* TargetContainer;

	// 실제 아이템을 가지고 있을 Grid
	UPROPERTY(meta = (BindWidget))
	UCPContainerGridWidget* ContainerGrid;

	// 컨테이너를 드래그 할 수 있는 Bar
	UPROPERTY(meta = (BindWidget))
	UBorder* DragBorder;

	bool bIsDraggingWindow = false;

	// Widget이 존재하면 바인딩, 없으면 nullptr
	UPROPERTY(meta = (BindWidgetOptional))
	UCPMoneyWidget* MoneyWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton;
};
