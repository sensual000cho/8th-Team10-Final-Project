#include "NPC/GA/CPGA_PotionReactionBase.h"
#include "NPC/CPBaseNPC.h"
#include "NPC/CPLabNPC.h"
#include "AbilitySystemComponent.h"

UCPGA_PotionReactionBase::UCPGA_PotionReactionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTagContainer InitialAssetTags;
	InitialAssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reaction.Potion")));
	SetAssetTags(InitialAssetTags);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.All")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Reaction.Potion")));
}

void UCPGA_PotionReactionBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (ImmunityTag.IsValid())
	{
		ActivationBlockedTags.AddTag(ImmunityTag);
	}
}

bool UCPGA_PotionReactionBase::ExtractImpactHitResult(const FGameplayEventData* TriggerEventData, FHitResult& OutHitResult)
{
	if (!TriggerEventData)
	{
		return false;
	}

	if (const FHitResult* HitResultPtr = TriggerEventData->ContextHandle.GetHitResult())
	{
		OutHitResult = *HitResultPtr;
		return true;
	}

	return false;
}

ACPBaseNPC* UCPGA_PotionReactionBase::GetOwningPotionNPC(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return ActorInfo ? Cast<ACPBaseNPC>(ActorInfo->AvatarActor.Get()) : nullptr;
}

ACPBaseNPC* UCPGA_PotionReactionBase::GetValidatedOwningPotionNPC(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	ACPBaseNPC* NPC = GetOwningPotionNPC(ActorInfo);
	if (!IsValid(NPC) || !HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return nullptr;
	}
	return NPC;
}

void UCPGA_PotionReactionBase::PrintPotionEventLog(const FGameplayEventData* TriggerEventData) const
{
	if (!TriggerEventData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Potion Event] TriggerEventData is NULL!"));
		return;
	}

	FString EventTagStr = TriggerEventData->EventTag.ToString();

	FString InstigatorName = TriggerEventData->Instigator ? TriggerEventData->Instigator->GetName() : TEXT("None");
	FString TargetName = TriggerEventData->Target ? TriggerEventData->Target->GetName() : TEXT("None");
	FString OptionalObjName = TriggerEventData->OptionalObject ? TriggerEventData->OptionalObject->GetName() : TEXT("None");

	FString ContextInstigator = TriggerEventData->ContextHandle.GetInstigator() ? TriggerEventData->ContextHandle.GetInstigator()->GetName() : TEXT("None");
	FString ContextCauser = TriggerEventData->ContextHandle.GetEffectCauser() ? TriggerEventData->ContextHandle.GetEffectCauser()->GetName() : TEXT("None");

	FString HitLocationStr = TEXT("None");
	if (const FHitResult* HitResult = TriggerEventData->ContextHandle.GetHitResult())
	{
		HitLocationStr = HitResult->ImpactPoint.ToString();
	}

	UE_LOG(LogTemp, Warning, TEXT("================ [ Potion Hit Event Data ] ================"));
	UE_LOG(LogTemp, Warning, TEXT("1. Event Tag      : %s"), *EventTagStr);
	UE_LOG(LogTemp, Warning, TEXT("2. Instigator     : %s"), *InstigatorName);
	UE_LOG(LogTemp, Warning, TEXT("3. Target         : %s"), *TargetName);
	UE_LOG(LogTemp, Warning, TEXT("4. Optional Obj   : %s"), *OptionalObjName);
	UE_LOG(LogTemp, Warning, TEXT("5. Context Handle : Instigator(%s), Causer(%s), HitLoc(%s)"), *ContextInstigator, *ContextCauser, *HitLocationStr);
	UE_LOG(LogTemp, Warning, TEXT("==========================================================="));
}