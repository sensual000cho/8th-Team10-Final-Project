// CPHUDBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CPHUDBase.generated.h"

class UCPUIManagerSubsystem;
class UCPCommonNotificationWidget;
class UCPForageableItemData;
class UCPNPCDataAsset;
/**
 * 모든 맵에서 공통적으로 적용하는 UI용 HUD
 */
UCLASS()
class CREATEPOTION_API ACPHUDBase : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
	// 캐싱된 UIManager
	UPROPERTY()
	TObjectPtr<UCPUIManagerSubsystem> CachedUIManager;
	
	// 각 맵에서 적용되는 메인 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainHUDWidgetClass;

#pragma region CommonPopupWidgets
	
public: 
	UFUNCTION()
	void HandleForageableCodexRegistered(UCPForageableItemData* ItemData);
	
	UFUNCTION()
	void HandleNPCCodexRegistered(UCPNPCDataAsset* NPCData);
	
protected:
	// 필요한 추가 위젯 켜기
	virtual void OnMainHUDWidgetCreated();
	
protected:
	// --- 모든 맵에서 적용되는 팝업 위젯 클래스 ---
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> QuestJournalWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> KeyGuideWidgetClass; 
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPCommonNotificationWidget> NotificationWidgetClass;
#pragma endregion
};
