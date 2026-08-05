// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "CPLabBell.generated.h"

class UStaticMeshComponent;

UCLASS()
class CREATEPOTION_API ACPLabBell : public AActor, public ICPInteractable
{
	GENERATED_BODY()
	
public:	
	ACPLabBell();
	
	//상호작용 인터페이스
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	//FTEXT로 상호작용 UI 프롬프트 - 채집하기, 벨 울리기 등등...
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	
	//lab 세션 시작을 GameMode에 요청
	UFUNCTION(BlueprintCallable, Category= "Lab|Bell")
	bool TryRingBell();
	


private:
	UPROPERTY(VisibleAnywhere, Category = "Lab|Bell")
	TObjectPtr<UStaticMeshComponent> BellMesh;
};
