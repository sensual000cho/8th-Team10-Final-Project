#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCore/Interface/CPInteractable.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "CPBaseNPC.generated.h"

class UCPNPCDataAsset;
class UAbilitySystemComponent;
class UCPNPCSubsystem;
class UCPGADataAsset;
class UCPNPCVisualComponent;

// (bActive: 적용/해제 여부, Magnitude: 배율/강도)
DECLARE_DELEGATE_TwoParams(FOnPotionVisualUpdateDelegate, bool /*bActive*/, float /*Magnitude*/);

UCLASS()
class CREATEPOTION_API ACPBaseNPC : public ACharacter, public ICPInteractable, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ACPBaseNPC();

	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() override;
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	virtual FName GetInteractionName_Implementation() override;

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

public:
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Potion")
	TObjectPtr<UCPGADataAsset> CommonAbilitySet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCPNPCVisualComponent> VisualComponent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UCPNPCDataAsset> NPCData;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	FVector BaseMeshScale = FVector::OneVector;

private:
	// 태그별로 바인딩된 비주얼 함수 맵
	TMap<FGameplayTag, FOnPotionVisualUpdateDelegate> VisualDelegateMap;
};