#include "Items/Potion/Debug/CPPotionImpactDummyReceiver.h"

#include "Components/BoxComponent.h"
#include "Data/CPPotionEffectRegistry.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

ACPPotionImpactDummyReceiver::ACPPotionImpactDummyReceiver()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	CollisionBox->SetBoxExtent(FVector(50.0f));
	CollisionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

bool ACPPotionImpactDummyReceiver::ReceivePotionImpact_Implementation(const FCPPotionImpactContext& Context)
{
	ReceivedImpactCount++;
	LastImpactContext = Context;

	// Registry에서 실제로 적용된 효과가 있을 때만 성공 처리
	const bool bApplied = IsValid(PotionEffectRegistry) && PotionEffectRegistry->TryApplyPotionEffects(Context);

	FString EffectTagsText;

	for (const FGameplayTag& EffectTag : Context.EffectTags)
	{
		if (!EffectTagsText.IsEmpty())
		{
			EffectTagsText += TEXT(", ");
		}

		EffectTagsText += EffectTag.ToString();
	}

	const FString Message = FString::Printf(TEXT("Potion Impact | Accepted: %s | Count: %d\nInstigator: %s\nSource: %s\nTarget: %s\nTags: [%s]\nPoint: %s | Normal: %s"), bApplied ? TEXT("true") : TEXT("false"), ReceivedImpactCount, *GetNameSafe(Context.Instigator.Get()), *GetNameSafe(Context.SourcePotion.Get()), *GetNameSafe(Context.TargetActor.Get()), *EffectTagsText, *Context.ImpactPoint.ToCompactString(), *Context.ImpactNormal.ToCompactString());

	// 포션 충돌 시 전달된 Context와 적용 결과를 화면에 출력하는 더미 검증 로그
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, bApplied ? FColor::Green : FColor::Red, Message);

	return bApplied;
}
