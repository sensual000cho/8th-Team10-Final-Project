#include "NPC/GA/CPGA_FartLaunch.h"
#include "NPC/CPBaseNPC.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

UCPGA_FartLaunch::UCPGA_FartLaunch()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	FloatDuration = 10.0f;
	LaunchForce = 1500.f;
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Immunity.Potion.FartLaunch")));

}

void UCPGA_FartLaunch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//PrintPotionEventLog(TriggerEventData);

	ACPBaseNPC* TargetCharacter = GetOwningPotionNPC(ActorInfo);

	if (!TargetCharacter || !HasAuthority(&ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	TargetCharacter->AddActorWorldOffset(FVector(0.f, 0.f, 10.f), false);

	if (UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement())
	{
		MovementComp->SetMovementMode(EMovementMode::MOVE_Falling);
	}

	UAbilityTask_ApplyRootMotionConstantForce* LaunchTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this,
		FName("FartRocketForce"),
		FVector(0.f, 0.f, 1.f), // 위(Z축)로 향하는 방향
		LaunchForce,            // 밀어붙이는 힘 
		FloatDuration,          // 지속 시간
		false,                  // 기존 속도에 더할지
		nullptr,                // 시간에 따른 힘 변화 커브 (없음)
		ERootMotionFinishVelocityMode::SetVelocity, // 끝났을 때의 속도 처리 방식
		FVector::ZeroVector,    // 끝나면 공중에 잠시 멈췄다가(Zero) 중력을 받고 떨어지게 만듦
		0.1f,
		false                   // 비행 중 중력 적용 여부
	);

	if (LaunchTask)
	{
		LaunchTask->OnFinish.AddDynamic(this, &UCPGA_FartLaunch::OnLaunchFinished);
		LaunchTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UCPGA_FartLaunch::OnLaunchFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}