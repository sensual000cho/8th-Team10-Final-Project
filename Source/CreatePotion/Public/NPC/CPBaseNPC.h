#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameCore/Interface/CPInteractable.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "CPBaseNPC.generated.h"

class UCPNPCDataAsset;
class UQuestManager;
class UAbilitySystemComponent;
class UGameplayAbility;
class UCPNPCSubsystem;
struct FGameplayEffectSpec;
struct FActiveGameplayEffectHandle; 
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
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "NPC|Potion")
	FName GetPotionNPCId() const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Potion")
	void RegisterPersistentPotionEffect(FGameplayTag EffectTag, int32 InDurationWorldMinutes, float Magnitude = 1.0f);

protected:
	//virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void InitializeFromDataAsset();

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void FitCapsuleToMesh(USkeletalMesh* NPCMesh);

	//void HandleGameplayEffectApplied(UAbilitySystemComponent* SourceASC, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveHandle);

	UFUNCTION()
	void HandlePotionEffectAppliedBroadcast(FName BroadcastNPCId, FGameplayTag EffectTag, int64 ExpiresAtWorldMinute, float Magnitude);

	UFUNCTION()
	void HandlePotionEffectExpiredBroadcast(FName BroadcastNPCId, FGameplayTag EffectTag);

	void CatchUpPersistentPotionEffects();

	UFUNCTION(BlueprintNativeEvent, Category = "NPC|Potion")
	void ApplyPotionEffectVisual(FGameplayTag EffectTag, bool bActive, float Magnitude);
	virtual void ApplyPotionEffectVisual_Implementation(FGameplayTag EffectTag, bool bActive, float Magnitude);

	UCPNPCSubsystem* GetNPCSubsystem() const;

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