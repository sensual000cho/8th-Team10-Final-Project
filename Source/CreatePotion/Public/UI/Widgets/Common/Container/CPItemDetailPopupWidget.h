// CPItemDetailPopupWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPItemDetailPopupWidget.generated.h"

class UImage;
class UTextBlock;
struct FCPItemInstance;

UCLASS()
class CREATEPOTION_API UCPItemDetailPopupWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()

public:
	void ShowDetail(const FCPItemInstance& Instance);
	void HideDetail();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override; // 추가 - 보이는 동안 마우스를 따라다님

private:
	UPROPERTY(EditDefaultsOnly, Category = "ItemDetail")
	float AutoHideDelay = 10.0f;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* LiquidImage; // 포션이 아니면 없어도 무방하게 Optional

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;
};
