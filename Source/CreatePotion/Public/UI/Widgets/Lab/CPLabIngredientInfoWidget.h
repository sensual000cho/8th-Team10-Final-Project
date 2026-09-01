// 재료 정보 카드의 공용 표시와 데이터 관찰 기능을 정의한다.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/CPLabTypes.h"
#include "UI/Widgets/Base/CPBaseUserWidget.h"
#include "CPLabIngredientInfoWidget.generated.h"

class UCPForageableItemData;
class UCPLabIngredientEffectRowWidget;
class ACPThrowablePropBase;
class UImage;
class UTextBlock;
class UVerticalBox;

// 재료 하나를 표시하는 공용 기반 위젯이다.
// 재료 렌더링과 Prop 변경 감지만 담당하며, 데이터 출처와 표시/숨김 정책은 자식이 결정한다.
 
UCLASS()
class CREATEPOTION_API UCPLabIngredientInfoWidget : public UCPBaseUserWidget
{
	GENERATED_BODY()

public:
	// 월드의 Prop를 관찰하며 작업 데이터가 바뀔 때마다 자동으로 다시 표시한다.
	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetIngredientProp(ACPThrowablePropBase* InProp);

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ClearObservedIngredient();

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ClearPreviewEffects();

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void ClearIngredientInfo();

	UFUNCTION(BlueprintPure, Category = "Lab|UI|Ingredient")
	bool HasIngredientInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Lab|UI|Ingredient")
	void SetHeaderText(const FText& InHeaderText);
	
protected:
	virtual void NativeConstruct() override;
	virtual void UnbindEvents() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Context;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_MaterialName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_MaterialIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_EffectRows;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|UI|Ingredient")
	TSubclassOf<UCPLabIngredientEffectRowWidget> EffectRowWidgetClass;

	// 화면에 표시할 이름만 덮어쓴다. 효과의 실제 식별자는 계속 GameplayTag를 사용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|UI|Ingredient")
	TMap<FGameplayTag, FText> EffectDisplayNames;

	// 운반 카드처럼 위젯은 남아 있지만 재료가 없는 상태에서 표시할 문구다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|UI|Ingredient")
	FText EmptyIngredientText = FText::FromString(TEXT("비어 있음"));
private:
	// 스냅샷과 Actor 관찰 경로가 최종적으로 공유하는 표시 진입점이다.
	void ApplyIngredientInfo(const FCPLabIngredientInstance& InIngredient);
	void RefreshObservedIngredient();

	// 다른 Prop으로 갈아타거나 위젯이 파괴될 때 이전 Prop의 델리게이트를 반드시 해제한다.
	void UnbindObservedIngredient();

	void RefreshWidget();
	void RefreshEmptyState();
	void RebuildEffectRows();
	FText GetEffectDisplayName(const FGameplayTag& EffectTag) const;

	UPROPERTY(Transient)
	FCPLabIngredientInstance Ingredient;

	// Actor의 생명주기를 UI가 연장하지 않도록 약한 참조로 관찰한다.
	UPROPERTY(Transient)
	TWeakObjectPtr<ACPThrowablePropBase> ObservedIngredientProp;
};
