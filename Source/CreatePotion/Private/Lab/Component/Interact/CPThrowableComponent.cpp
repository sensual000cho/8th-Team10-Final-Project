#include "Lab/Component/Interact/CPThrowableComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Lab/Actor/CPThrowablePropBase.h"

UCPThrowableComponent::UCPThrowableComponent()
{
	TargetName = FName(TEXT("Prop"));
	InteractionPrompt = FText::FromString(TEXT("들기"));
}

bool UCPThrowableComponent::CanInteract() const
{
	const ACPThrowablePropBase* Prop = Cast<ACPThrowablePropBase>(GetOwner());
	return IsValid(Prop) && Prop->CanBePickedUp();
}

bool UCPThrowableComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor)) return false;

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Interactor);
	if (!IsValid(AbilitySystem)) return false;

	static const FGameplayTag PickupEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Carry.Pickup")));
	FGameplayEventData EventData;
	EventData.EventTag = PickupEventTag;
	EventData.Instigator = Interactor;
	EventData.Target = GetOwner();

	AbilitySystem->HandleGameplayEvent(PickupEventTag, &EventData);
	return true;
}

void UCPThrowableComponent::SetTargetName(FName InTargetName)
{
	TargetName = InTargetName;
	RefreshUI();
}
