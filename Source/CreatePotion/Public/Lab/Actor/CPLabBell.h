#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameFramework/Actor.h"
#include "CPLabBell.generated.h"

class UStaticMeshComponent;

// 플레이어가 공방 포션 세션을 시작할 때 사용하는 벨
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
	//아직 포션 세션이 시작되지 않았을 때만 상호작용 허용
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	
	//lab 세션 시작을 GameMode에 요청
	UFUNCTION(BlueprintCallable, Category= "Lab|Bell")
	bool TryRingBell();


	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Lab|Bell")
	TObjectPtr<UStaticMeshComponent> BellMesh;
};
