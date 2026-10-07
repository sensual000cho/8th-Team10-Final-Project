// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Quest/QuestTypes.h"
#include "CPQuestMarkerComponent.generated.h"

class UCPInventoryComponent;

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPQuestMarkerComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UCPQuestMarkerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Quest|Marker")
	void UpdateQuestMarker();

	// 튜토리얼처럼 단계마다 대상 퀘스트가 바뀌는 경우 사용
	UFUNCTION(BlueprintCallable, Category = "Quest|Marker")
	void SetTrackedQuest(FName QuestID);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleQuestUpdated(FName QuestID, EQuestState NewState);

	UFUNCTION()
	void HandleInventoryUpdated();

	void TryBindInventory();

	UPROPERTY(Transient)
	FName TrackedQuestID = NAME_None;

	TWeakObjectPtr<UCPInventoryComponent> BoundInventory;
};