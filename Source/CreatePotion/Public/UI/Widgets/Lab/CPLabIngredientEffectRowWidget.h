// 재료 효과 한 줄과 예상 변화 표시 상태를 정의한다.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPLabIngredientEffectRowWidget.generated.h"

class UTextBlock;
class UWidget;

UENUM(BlueprintType)
enum class ECPLabIngredientEffectDeltaState : uint8
{
	None,
	Increased,
	Decreased,
	Unchanged
};

// 재료 효과 하나를 WBP에서 각각 꾸밀 수 있는 블록으로 표시한다.
// 색상, 배경, 화살표 이미지 같은 시각 표현은 WBP가 담당한다.
UCLASS()
class CREATEPOTION_API UCPLabIngredientEffectRowWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetEffectData(
		const FText& InEffectName,
		int32 InCurrentLevel,
		bool bInHasPreview,
		int32 InPreviewLevel);

	UFUNCTION(BlueprintPure, Category = "Lab|UI|Ingredient")
	ECPLabIngredientEffectDeltaState GetDeltaState() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Lab|UI|Ingredient", meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_EffectName;

	UPROPERTY(BlueprintReadOnly, Category = "Lab|UI|Ingredient", meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_CurrentLevel;

	// HorizontalBox, Overlay, Border 등 예상값 묶음을 한 번에 숨길 수 있는 부모 위젯을 바인딩한다.
	UPROPERTY(BlueprintReadOnly, Category = "Lab|UI|Ingredient", meta = (BindWidget))
	TObjectPtr<UWidget> PreviewGroup;

	UPROPERTY(BlueprintReadOnly, Category = "Lab|UI|Ingredient", meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PreviewLevel;

	/**
	 * WBP_CP_LabIngredientEffectRow에서 구현한다.
	 * 변화 상태별 화살표, 예상 레벨, 배경 등의 스타일만 Blueprint가 결정한다.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lab|UI|Ingredient", meta = (DisplayName = "Apply Delta Style"))
	void BP_ApplyDeltaStyle(ECPLabIngredientEffectDeltaState InDeltaState);

private:
	// 모든 레벨 문구가 동일한 "Lv. N" 형식을 사용하도록 한곳에서 생성한다.
	static FText FormatLevel(int32 Level);

	UPROPERTY(Transient)
	ECPLabIngredientEffectDeltaState DeltaState = ECPLabIngredientEffectDeltaState::None;
};
