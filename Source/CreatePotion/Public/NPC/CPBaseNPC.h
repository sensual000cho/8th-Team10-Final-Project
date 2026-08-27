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

	//NPC의 ASC 반환
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//DataAsset 기반 NPCName 반환
	UFUNCTION(BlueprintPure, Category = "NPC|Potion")
	virtual FName GetPotionNPCId() const;

	//NPC에게 적용된 포션 지속 효과를 NPC 서브시스템에 등록
	UFUNCTION(BlueprintCallable, Category = "NPC|Potion")
	void RegisterPersistentPotionEffect(FGameplayTag EffectTag, int32 InDurationWorldMinutes, float Magnitude = 1.0f);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//NPCData에 설정된 메쉬, 애니메이션 등을 적용
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void InitializeFromDataAsset();

	//NPC 캡슐 컴포넌트 반경과 높이 적용
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void FitCapsuleToMesh(USkeletalMesh* NPCMesh);

	//CPNPCSubsystem에서 특정 효과가 적용/갱신되었을 때 브로드캐스트를 수신하는 핸들러
	UFUNCTION()
	void HandlePotionEffectAppliedBroadcast(FName BroadcastNPCId, FGameplayTag EffectTag, int64 ExpiresAtWorldMinute, float Magnitude);

	//CPNPCSubsystem에서 특정 효과의 지속 시간이 만료되었을 때 브로드캐스트를 수신하는 핸들러
	UFUNCTION()
	void HandlePotionEffectExpiredBroadcast(FName BroadcastNPCId, FGameplayTag EffectTag);

	//레벨 이동에 서브시스템에 저장되어 있던 활성 효과들을 불러와 비주얼에 동기화
	void CatchUpPersistentPotionEffects();

	//효과 태그에 맞춰 외형적인 변화를 적용
	UFUNCTION(BlueprintNativeEvent, Category = "NPC|Potion")
	void ApplyPotionEffectVisual(FGameplayTag EffectTag, bool bActive, float Magnitude);
	virtual void ApplyPotionEffectVisual_Implementation(FGameplayTag EffectTag, bool bActive, float Magnitude);

	UCPNPCSubsystem* GetNPCSubsystem() const;

	//commonAbilitySet에 정의된 GA 목록을 NPC의 ASC에 부여
	void GrantPotionReactionAbilities();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UCPNPCDataAsset> NPCData;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Potion")
	TObjectPtr<UCPGADataAsset> CommonAbilitySet;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, Category = "NPC|Potion")
	FGameplayTagContainer PersistentPotionEffectTags;

	UPROPERTY()
	FVector BaseMeshScale = FVector::OneVector;
};