// CPPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "Types/CPContainerTypes.h"
#include "CPPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UCPItemContainerComponent;
class UCPInventoryComponent;
class UCPContainerMainWidget;
class UCPContainerContextBase;

UCLASS()
class CREATEPOTION_API ACPPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

#pragma region Container
public:
	// 대상 컨테이너의 ContainerUIClass를 읽어 UI를 열고 바인딩하는 범용 함수.
	// Lab, Storage, Shop 등 어떤 외부형 컨테이너든 동일하게 처리 가능.
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void OpenExternalContainerUI(UCPItemContainerComponent* InTargetContainer);

	// 현재 열려있는 외부 컨테이너 UI를 닫는 범용 함수
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void CloseExternalContainerUI();

	// 상호작용(F키 등)에서 바로 호출하기 좋은 토글 버전.
	// 이미 같은 컨테이너가 열려있으면 닫고, 아니면 엶.
	UFUNCTION(BlueprintCallable, Category = "Container|UI")
	void ToggleExternalContainerUI(UCPItemContainerComponent* InTargetContainer);

private:
	void SetContextHandlerForTargetContext(EUITargetContext InTargetContext);

	// Inventory Component 캐싱을 시도할 함수
	void TryCacheInventoryComponent();

public:
	// 현재 상호작용 중인 컨테이너
	// TODO[Container] : F키 또는 컨테이너 이용 시 이 값을 해당 ItemContainerComponent로 설정해주어야 합니다.
	UPROPERTY(BlueprintReadWrite, Category = "Container")
	UCPItemContainerComponent* CurrentInteractingContainer = nullptr;

	// Context를 처리할 Handler
	UPROPERTY()
	UCPContainerContextBase* CurrentContextHandler;

	// 캐싱할 Inventory Component
	UPROPERTY()
	UCPInventoryComponent* CachedInventoryComponent;

protected:
	// 현재 화면에 떠 있는 외부 컨테이너 UI 인스턴스 (Lab, Storage, Shop 공용)
	UPROPERTY()
	UCPContainerMainWidget* CurrentContainerUIInstance;

private:
	FTimerHandle InventoryCacheRetryHandler;
#pragma endregion
};