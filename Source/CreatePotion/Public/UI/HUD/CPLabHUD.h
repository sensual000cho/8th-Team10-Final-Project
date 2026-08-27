// CPLabHUD.h

#pragma once

#include "CoreMinimal.h"
#include "UI/HUD/CPHUDBase.h"
#include "CPLabHUD.generated.h"


UCLASS()
class CREATEPOTION_API ACPLabHUD : public ACPHUDBase
{
	GENERATED_BODY()
	
public:
	void OnMainHUDWidgetCreated();
	
	// TODO: 나중에 포션 제작 UI 흐름 이쪽으로 옮기기
	// 공방 포션 제작 UI 플로우 시작 함수
	UFUNCTION(BlueprintCallable, Category = "UI Flow")
	void StartLabCraftingFlow();

public:
	UFUNCTION()
	void HandleQuestJournalToggle();
};
