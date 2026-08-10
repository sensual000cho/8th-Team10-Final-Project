// CPHUDBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CPHUDBase.generated.h"

UCLASS()
class CREATEPOTION_API ACPHUDBase : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainHUDWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> MainHUDWidget;
	
	// 필요한 추가 위젯 켜기
	virtual void OnMainHUDWidgetCreated();
};
