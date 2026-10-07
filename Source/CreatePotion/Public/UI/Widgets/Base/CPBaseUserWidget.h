// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CPBaseUserWidget.generated.h"

class UCPUIManagerSubsystem;
/**
 * 
 */
UCLASS()
class CREATEPOTION_API UCPBaseUserWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UCPInventoryComponent;
	
public:
	// ESC 콜백함수
	virtual bool HandleEscape();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	// 바인딩, 언바인딩 함수
	// 자식 클래스에서 override해서 사용
	virtual void BindEvents();
	virtual void UnbindEvents();
	
	// ESC로 닫을 수 있는지 여부 반환
	bool CanCloseByEscape() const { return bCanCloseByEscape; }

	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "UI|Widget")
	int32 ZOrder = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Sound")
	class USoundBase* OpenSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Sound")
	class USoundBase* CloseSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Input")
	bool bCanCloseByEscape = false;
protected:
	// 캐싱된 UIMangerSubsystem
	UPROPERTY()
	TObjectPtr<UCPUIManagerSubsystem> CachedUIManager;
};
