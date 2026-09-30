#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "CPBaseFieldActor.generated.h"

// 사운드 구조체
USTRUCT(BlueprintType)
struct FCPFieldSoundSet
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> StartSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> LoopSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> EndSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundAttenuation> Attenuation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundConcurrency> Concurrency;
};

class UGameplayAbility;
// 필드레벨에서 포션으로 상호작용 가능한 액터의 베이스
UCLASS()
class CREATEPOTION_API ACPBaseFieldActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	ACPBaseFieldActor();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable)
	virtual bool PotionReaction(const FGameplayTag& EventTag, AActor* InstigatorActor = nullptr);
	
	UFUNCTION(BlueprintCallable)
	void PlayStartSound();
	
	UFUNCTION(BlueprintCallable)
	void StartLoopSound();
	
	UFUNCTION(BlueprintCallable)
	void StopLoopSound();

protected:
	virtual void BeginPlay() override;
	
	void GrantPotionReactionAbilities();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAudioComponent> LoopAudioComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	// 액터에서 사용할 GA들
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> PotionReactionAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	FCPFieldSoundSet SoundSet;
};
