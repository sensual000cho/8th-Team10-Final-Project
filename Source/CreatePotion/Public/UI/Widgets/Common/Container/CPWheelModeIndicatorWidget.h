// CPWheelModeIndicatorWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "CPWheelModeIndicatorWidget.generated.h"

class UTextBlock;
enum class EWheelInputMode : uint8;

UCLASS()
class CREATEPOTION_API UCPWheelModeIndicatorWidget : public UCPBaseFixedWidget
{
	GENERATED_BODY()
	
public:
	// 모드가 바뀔 때마다 PC가 호출 - 텍스트를 갱신하고 3초짜리 표시 타이머를 (재)시작
	UFUNCTION(BlueprintCallable, Category = "WheelMode")
	void ShowMode(EWheelInputMode NewMode);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HideIndicator();

private:
	FTimerHandle HideTimerHandler;

	UPROPERTY(EditDefaultsOnly, Category = "WheelMode")
	float DisplayDuration = 3.0f;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WheelModeText;
};
