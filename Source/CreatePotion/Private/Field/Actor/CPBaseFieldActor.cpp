#include "Field/Actor/CPBaseFieldActor.h"
#include "AbilitySystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

ACPBaseFieldActor::ACPBaseFieldActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	LoopAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LoopAudioComponent"));
	
	LoopAudioComponent->SetupAttachment(Root);
	LoopAudioComponent->bAutoActivate = false;
}

UAbilitySystemComponent* ACPBaseFieldActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool ACPBaseFieldActor::PotionReaction(const FGameplayTag& EventTag, AActor* InstigatorActor)
{
	if (!AbilitySystemComponent) return false;
	
	FGameplayEventData EventData;
	
	EventData.EventTag = EventTag;
	EventData.Instigator = InstigatorActor;
	EventData.Target = this;
	
	const int32 ActivatedAbilityCount = AbilitySystemComponent->HandleGameplayEvent(EventTag, &EventData);
	
	return ActivatedAbilityCount > 0;
}

void ACPBaseFieldActor::PlayStartSound()
{
	if (!SoundSet.StartSound) return;
	
	UGameplayStatics::PlaySoundAtLocation(
		this,
		SoundSet.StartSound,
		GetActorLocation(),
		1.f,
		1.f,
		0.f,
		SoundSet.Attenuation,
		SoundSet.Concurrency);
}

void ACPBaseFieldActor::StartLoopSound()
{
	if (!SoundSet.LoopSound || !LoopAudioComponent) return;
	
	LoopAudioComponent->SetSound(SoundSet.LoopSound);
	
	if (SoundSet.Attenuation)
	{
		LoopAudioComponent->AttenuationSettings = SoundSet.Attenuation;
	}
	
	LoopAudioComponent->Play();
}

void ACPBaseFieldActor::StopLoopSound()
{
	if (LoopAudioComponent && LoopAudioComponent->IsPlaying())
	{
		LoopAudioComponent->FadeOut(0.3f, 0.f);
	}
	
	if (SoundSet.EndSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
	this,
		SoundSet.EndSound,
		GetActorLocation(),
		1.f,
		1.f,
		0.f,
		SoundSet.Attenuation,
		SoundSet.Concurrency);
	}
}

void ACPBaseFieldActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!AbilitySystemComponent) return;
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
	GrantPotionReactionAbilities();
}

void ACPBaseFieldActor::GrantPotionReactionAbilities()
{
	if (!AbilitySystemComponent) return;
	
	for (const TSubclassOf<UGameplayAbility>& Abilityclass : PotionReactionAbilities)
	{
		if (!Abilityclass) continue;
		
		FGameplayAbilitySpec AbilitySpec(Abilityclass, 1, INDEX_NONE, this);
		
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}