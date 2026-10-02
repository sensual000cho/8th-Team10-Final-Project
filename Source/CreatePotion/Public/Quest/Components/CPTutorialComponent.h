#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "Quest/QuestTypes.h"
#include "CPTutorialComponent.generated.h"

class UCPNPCDialogueWidget;

USTRUCT(BlueprintType)
struct FCPTutorialStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName QuestID;

	// false: 수락 단계, true: 완료 단계
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsCompletionStep = false;

	// 이 단계에서 액터가 서 있을 위치 (레벨에 배치한 TargetPoint)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<AActor> TargetPoint;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPTutorialComponent : public UCPInteractableComponent
{
	GENERATED_BODY()

public:
	UCPTutorialComponent();

	virtual bool CanInteract() const override;
	virtual bool TryInteract(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleQuestUpdated(FName QuestID, EQuestState NewState);

	void RefreshStep();

private:
	// 레벨에 배치한 인스턴스마다 단계 설정 가능 (진행 순서대로 입력)
	UPROPERTY(EditAnywhere, Category = "Quest")
	TArray<FCPTutorialStep> Steps;

	UPROPERTY(Transient)
	FName ActiveQuestID = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCPNPCDialogueWidget> DialogueWidgetClass;
};