// CPPlayerAbilitySystemComponent.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CPPlayerAbilitySystemComponent.generated.h"

class UGameplayAbility;

/*
 * 플레이어 캐릭터용 AbilitySystemComponent.
 * 캐릭터가 BP로만 구성되어 있으므로 ActorInfo 초기화와
 * 시작 Gameplay Ability 부여를 컴포넌트 내부에서 처리.
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPPlayerAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UCPPlayerAbilitySystemComponent();

	// Character를 OwnerActor와 AvatarActor로 사용해 ASC를 초기화
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void InitializeAbilitySystem();

protected:
	virtual void BeginPlay() override;

	// 게임 시작 시 플레이어 ASC에 부여할 Ability 목록(배열)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", meta = (ClampMin = "1"))
	int32 StartupAbilityLevel;

private:
	void GrantStartupAbilities();
};
