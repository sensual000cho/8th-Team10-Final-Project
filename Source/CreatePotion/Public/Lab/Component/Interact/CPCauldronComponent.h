// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Lab/CPLabTypes.h"
#include "Lab/Component/CPLabInteractActionComponent.h"
#include "CPCauldronComponent.generated.h"

class ACPAlchemyProp;
class UPrimitiveComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CREATEPOTION_API UCPCauldronComponent : public UCPLabInteractActionComponent
{
	GENERATED_BODY()

public:
	// Life Cycle
	UCPCauldronComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// 상호작용
	virtual bool ExecuteInteraction(AActor* Interactor) override;
	virtual bool CanExecuteInteraction(AActor* Interactor) const override;
	
	// Getter
	UFUNCTION(BlueprintPure, Category = "Lab|Interaction")
	TArray<FGameplayTag> GetEffectTags() const;
	
	UFUNCTION(BlueprintPure, Category = "Lab|Interaction")
	TArray<FCPLabIngredientInstance> GetIngredientInstance() const;
	
private:
	// PotionSession에서 Held 재료를 꺼내 투입
	bool AddHeldProp(AActor* Interactor);

	// F 방식과 투척 방식이 공통으로 사용하는 실제 투입 처리
	bool AddProp(ACPAlchemyProp* Prop);

	// 현재 세션과 가마솥이 해당 재료를 받을 수 있는지 검사
	bool CanAcceptProp(const ACPAlchemyProp* Prop) const;

	// 슬롯이 모두 찼다면 포션 제작
	bool ConfirmPotionIfReady();
	
	// 넣은 재료로 포션 제작
	bool ConfirmPotion();
	
	// SpawnMesh의 상단 Spawn 위치 계산
	FTransform MakePotionTransform() const;
	
	// 가마솥 내부 Trigger 진입 처리
	UFUNCTION()
	void HandleIngredientOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	// UI 도입 or PR확인 이후 삭제
	UFUNCTION(BlueprintCallable, Category = "Lab|Interaction|Debug")
	void DebugPrintSlots() const;
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Lab|Interaction")
	TArray<FCPLabIngredientInstance> IngredientInstances;

	UPROPERTY(EditAnywhere, Category = "Lab|Interaction")
	int32 MaxSlotCount;

	// 가마솥 내부에 배치한 Box/Sphere Collision 지정
	UPROPERTY(EditAnywhere, Category = "Lab|Interaction")
	FComponentReference IngredientTrigger;

	UPROPERTY(EditAnywhere, Category = "Lab|Potion")
	FComponentReference PotionSpawnMesh;

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> BoundIngredientTrigger;
};
