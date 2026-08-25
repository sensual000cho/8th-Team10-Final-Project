// CPHandHeldItemWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "Types/CPContainerTypes.h"
#include "CPHandHeldItemWidget.generated.h"

class UImage;
class UTextBlock;
class USizeBox;
class UCPItemContainerComponent;

UCLASS()
class CREATEPOTION_API UCPHandHeldItemWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()
	
public:
	void TryBindHandContainer(UCPItemContainerComponent* InHandContainer);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void UpdatePositionToMouse();

	float CalcCellCenterOffset() const;

private:
	UFUNCTION()
	void OnHandContainerUpdated();

	UPROPERTY()
	UCPItemContainerComponent* HandContainer;

	UPROPERTY(meta = (BindWidget))
	USizeBox* RootSizeBox;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StackText;
};
