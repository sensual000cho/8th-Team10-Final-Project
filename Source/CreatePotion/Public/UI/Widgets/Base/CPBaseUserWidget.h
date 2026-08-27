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
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	// 바인딩, 언바인딩 함수
	// 자식 클래스에서 override해서 사용
	virtual void BindEvents();
	virtual void UnbindEvents();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Sound")
	class USoundBase* OpenSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI|Sound")
	class USoundBase* CloseSound;
	
protected:
	// 캐싱된 UIMangerSubsystem
	UPROPERTY()
	TObjectPtr<UCPUIManagerSubsystem> CachedUIManager;
};
