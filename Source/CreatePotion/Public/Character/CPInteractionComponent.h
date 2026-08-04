// CPInteractionComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPromptChanged, FText, Prompt);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void TryInteract();
	
	UPROPERTY(BlueprintAssignable)
	FOnPromptChanged OnPromptChanged;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TraceDistance = 200.f; // 감지 거리
	
private:
	void PerformTrace(); // 타이머로 주기 실행
    
	FTimerHandle TraceTimerHandle; // 타이머 핸들
	
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTarget; // 감지된 대상
	
};
