// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widgets/Base/CPBaseFixedWidget.h"
#include "UI/Widgets/Base/CPBasePopupWidget.h"
#include "CPLabIngredientSelectWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIngredientConfirmedDelegate);

class UButton;
class UCPItemContainerComponent;
class UCPContainerMainWidget;
class UCPForageableItemData;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPLabIngredientSelectWidget : public UCPBasePopupWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;
	void BindEvents() override;

	
protected:
	// 바인딩
	UPROPERTY(meta=(BindWidget))
	UButton* Button_Confirm;
	
	// UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category = "UI")
	// UCPContainerMainWidget* InventoryWidget;

	UPROPERTY(meta=(BindWidget), BlueprintReadWrite, Category = "UI")
	UCPContainerMainWidget* SelectSlotWidget;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnConfirmClicked();
public:
	// 태그 확정 완료 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnIngredientConfirmedDelegate OnIngredientConfirmed;
	
public:
	// 테스트용 아이템 저장 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Test")
	TArray<UCPForageableItemData*> TestItemDatas;
	
	// 테스트용 아이템 추가 함수
	void AddTestItems();
	
	

};
