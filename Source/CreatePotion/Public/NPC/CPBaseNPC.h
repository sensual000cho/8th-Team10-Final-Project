#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "CPBaseNPC.generated.h"

class UCPNPCDataAsset;
class UAbilitySystemComponent;
class UCPNPCSubsystem;
class UCPGADataAsset;
class UCPNPCVisualComponent;
class UCPNPCMoveComponent;
class UCPInteractableComponent;
class UWidgetComponent;

// (bActive: 적용/해제 여부, Magnitude: 배율/강도)
DECLARE_DELEGATE_TwoParams(FOnPotionVisualUpdateDelegate, bool /*bActive*/, float /*Magnitude*/);

UCLASS()
class CREATEPOTION_API ACPBaseNPC : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACPBaseNPC();

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	virtual void HandleInteract(AActor* Interactor);

	UFUNCTION()
	virtual void HandleInteractionStarted(AActor* Interactor);

	virtual FText GetInteractionPrompt() const;
	virtual bool CanInteract(AActor* Interactor) const;
	virtual FName GetInteractionName() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Interaction")
	FORCEINLINE UCPInteractableComponent* GetInteractableComponent() const { return InteractableComponent; }
	// NPC의 ASC 반환
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// DataAsset 기반 NPCName 반환
	UFUNCTION(BlueprintPure, Category = "NPC|Potion")
	virtual FName GetPotionNPCId() const;

	// NPC에게 적용된 포션 지속 효과를 NPC 서브시스템에 등록
	UFUNCTION(BlueprintCallable, Category = "NPC|Potion")
	void RegisterPersistentPotionEffect(FGameplayTag EffectTag, int32 InDurationWorldMinutes, float Magnitude = 1.0f);

	// CPNPCSubsystem이 효과를 적용할 때 직접 호출
	virtual void HandlePotionEffectApplied(FGameplayTag EffectTag, int64 ExpiresAtWorldMinute, float Magnitude);

	// CPNPCSubsystem이 효과 만료 시 직접 호출
	virtual void HandlePotionEffectExpired(FGameplayTag EffectTag);

	UFUNCTION(BlueprintPure, Category = "NPC")
	FVector GetBaseMeshScale() const { return BaseMeshScale; }

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void SetNPCData(UCPNPCDataAsset* InNPCData);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	virtual void SetNPCActiveState(bool bActive);
	
	UFUNCTION(BlueprintPure, Category = "NPC|Data")
	const UCPNPCDataAsset* GetNPCData() const { return NPCData; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	FORCEINLINE UCPNPCVisualComponent* GetVisualComponent() const { return VisualComponent; }

	// GA가 비주얼 갱신 콜백을 등록/해제
	void RegisterVisualDelegate(FGameplayTag EffectTag, const FOnPotionVisualUpdateDelegate& Delegate);
	void UnregisterVisualDelegate(FGameplayTag EffectTag);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void InitializeFromDataAsset();

	// 레벨 이동/스폰 시 서브시스템에 저장되어 있던 활성 효과들을 불러와 태그/비주얼 동기화
	void CatchUpPersistentPotionEffects();

	// 등록된 델리게이트를 찾아 비주얼 갱신 실행
	UFUNCTION(BlueprintNativeEvent, Category = "NPC|Potion")
	void ApplyPotionEffectVisual(FGameplayTag EffectTag, bool bActive, float Magnitude);
	virtual void ApplyPotionEffectVisual_Implementation(FGameplayTag EffectTag, bool bActive, float Magnitude);

	UCPNPCSubsystem* GetNPCSubsystem() const;
	void GrantPotionReactionAbilities();
	void RegisterCodex();

	UFUNCTION(BlueprintCallable, Category = "NPC|Interaction")
	virtual void RefreshInteractionState();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TObjectPtr<UCPGADataAsset> CommonAbilitySet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCPNPCVisualComponent> VisualComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UCPNPCDataAsset> NPCData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCPNPCMoveComponent> MoveComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCPInteractableComponent> InteractableComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TObjectPtr<UWidgetComponent> InteractionWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction", meta = (ClampMin = "0.02", Units = "s"))
	float InteractionRefreshInterval = 0.15f;

	UPROPERTY()
	FVector BaseMeshScale = FVector::OneVector;

private:
	// 태그별로 바인딩된 비주얼 함수 맵
	TMap<FGameplayTag, FOnPotionVisualUpdateDelegate> VisualDelegateMap;

	FTimerHandle InteractionRefreshTimerHandle;

#pragma region Route Debug
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	void DrawDebugPatrolRoute();
#endif
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowPatrolRouteInEditor = false;

	UPROPERTY(EditAnywhere, Category = "Debug")
	FColor PatrolRouteColor = FColor::Emerald;

	UPROPERTY(EditAnywhere, Category = "Debug", meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "20.0"))
	float RouteLineThickness = 10.0f;

#pragma endregion Route Debug

};