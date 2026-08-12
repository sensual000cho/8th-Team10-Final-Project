// CPLabHUD.h

#pragma once

#include "CoreMinimal.h"
#include "UI/HUD/CPHUDBase.h"
#include "CPLabHUD.generated.h"

class UCPTagSelectionWidget;
class UCPLabIngredientSelectWidget;
class UCPTagRangeWidget;

UCLASS()
class CREATEPOTION_API ACPLabHUD : public ACPHUDBase
{
	GENERATED_BODY()
	
public:
	// TODO: 나중에 포션 제작 UI 흐름 이쪽으로 옮기기
	// 공방 포션 제작 UI 플로우 시작 함수
	UFUNCTION(BlueprintCallable, Category = "UI Flow")
	void StartLabCraftingFlow();
	
public:
	// --- 위젯 델리게이트 바인딩 함수 ---
	void BindTagSelectionWidget(UCPTagSelectionWidget* TargetWidget);
	void BindTagRangeWidget(UCPTagRangeWidget* TargetWidget);
	void BindLabIngredientSelectWidget(UCPLabIngredientSelectWidget* TargetWidget);
	
	
public:
	UFUNCTION()
	void HandleTagRangeConfirmed();
	
	UFUNCTION()
	void HandleTagSelectionConfirmed(const FTagSelectionData& SelectionData);
	
	// UFUNCTION()
	// void HandleIngredientSelectionConfirmed(const );
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPTagSelectionWidget> TagSelectionWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPTagRangeWidget> TagRangeWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPLabIngredientSelectWidget> LabIngredientSelectWidgetClass;

};
