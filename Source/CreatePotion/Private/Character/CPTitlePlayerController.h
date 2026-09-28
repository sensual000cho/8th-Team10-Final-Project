// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPTitlePlayerController.generated.h"

class ACPBookMenuActor;
/**
 * 타이틀용 플레이어 컨트롤러
 */
UCLASS()
class CREATEPOTION_API ACPTitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Title")
	void StartTitleBlend();
	
	// 태그로 액터를 찾는 헬퍼함수
	AActor* FindActorByTag(FName Tag) const;
	
	// 카메라 블렌드 종료 콜백함수
	void HandleCameraBlendComplete();
	
protected:
	virtual void BeginPlay() override;
	
private:
	// 타이틀 메뉴 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "Title")
	TSubclassOf<UUserWidget> TitleMenuClass;
	
	// 메뉴 이전 카메라 태그
	UPROPERTY(EditDefaultsOnly, Category = "Title")
	FName PreTitleCameraTag = TEXT("PreTitleCam");
	
	// 책 내부 메뉴 위젯을 비추는 카메라 태그
	UPROPERTY(EditDefaultsOnly, Category = "Title")
	FName TitleCameraTag = TEXT("TitleCam");
	
	UPROPERTY(EditAnywhere, Category = "Title")
	TObjectPtr<AActor> PreTitleCamera;
	
	// 클로즈업 카메라 
	UPROPERTY(EditAnywhere, Category = "Title")
	TObjectPtr<AActor> TitleCamera;
	
	UPROPERTY(EditAnywhere, Category = "Title")
	float BlendTime = 2.f;
	
	// 타이틀 메뉴 위젯
	UPROPERTY()
	TObjectPtr<UUserWidget> TitleMenu;
	
	UPROPERTY()
	TObjectPtr<ACPBookMenuActor> BookMenuActor;
	
	FDelegateHandle BlendHandle;
	FTimerHandle FallbackTimer;
	bool bMenuLocked = true;
	bool bBlendStarted = false;
	
};
