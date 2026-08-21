#include "Items/Potion/Debug/CPPotionImpactDummyReceiver.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "GameplayEffect.h"

ACPPotionImpactDummyReceiver::ACPPotionImpactDummyReceiver()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	CollisionBox->SetBoxExtent(FVector(50.0f));
	CollisionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* ACPPotionImpactDummyReceiver::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACPPotionImpactDummyReceiver::BeginPlay()
{
	Super::BeginPlay();

	// 정식 NPC나 Prop도 ASC 사용 전에 Owner와 Avatar를 초기화한다.
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	// 더미의 면역 성공과 차단 테스트용이며 정식 상태 부여 방식은 대상 ASC 정책을 따른다.
	AbilitySystemComponent->AddLooseGameplayTags(InitialOwnedTags);
	AbilitySystemComponent->OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ACPPotionImpactDummyReceiver::HandleGameplayEffectApplied);

	if (HasAuthority())
	{
		// 정식 대상의 초기 Ability 부여 과정을 에디터 배열로 단순하게 재현한다.
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : InitialAbilities)
		{
			if (AbilityClass)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
			}
		}
	}
}

void ACPPotionImpactDummyReceiver::HandleGameplayEffectApplied(UAbilitySystemComponent* SourceAbilitySystem, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	ReceivedGameplayEffectCount++;
	LastAppliedEffectTags.Reset();
	EffectSpec.GetAllAssetTags(LastAppliedEffectTags);

	FString EffectTagsText;

	for (const FGameplayTag& EffectTag : LastAppliedEffectTags)
	{
		if (!EffectTagsText.IsEmpty())
		{
			EffectTagsText += TEXT(", ");
		}

		EffectTagsText += EffectTag.ToString();
	}

	const FString Message = FString::Printf(TEXT("Potion GAS Effect | Applied: true | Count: %d\nSource ASC: %s\nTarget: %s\nGameplayEffect: %s\nTags: [%s]"), ReceivedGameplayEffectCount, *GetNameSafe(SourceAbilitySystem), *GetName(), *GetNameSafe(EffectSpec.Def), *EffectTagsText);

	// 대상 ASC가 실제로 수용한 GameplayEffect와 포션 효과 태그를 화면에 출력하는 더미 검증 로그.
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
}
