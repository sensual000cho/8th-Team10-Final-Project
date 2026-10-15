// CPRespawnComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPRespawnComponent.generated.h"

class UCPInteractionComponent;
class ACharacter;
class APlayerStart;
class UCPHealthComponent;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCPOnRespawnCompleted);

UCLASS(ClassGroup = (Character), meta = (BlueprintSpawnableComponent))
class CREATEPOTION_API UCPRespawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<class USoundBase> GameOverSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<class USoundBase> RespawnSound;


	UCPRespawnComponent();

	// 캐릭터 BP에서 사망 몽타주를 재생하는 Mesh 지정
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetRespawnAnimationMesh(USkeletalMeshComponent* InMesh);

	// 타이머에서 호출. 필요하면 BP에서도 직접 호출 가능
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	bool Respawn();

	// 부활 성공 후 바인딩 용 델리게이트 함수(UI 등)
	UPROPERTY(BlueprintAssignable, Category = "Respawn|Event")
	FCPOnRespawnCompleted OnRespawnCompleted;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleOwnerDeath(AActor* DamageCauser);
	void HandleRespawnTimer();
	APlayerStart* FindRespawnStart() const;

private:
	UPROPERTY(EditAnywhere, Category = "Respawn", meta = (ClampMin = "0.01", Units = "s"))
	float RespawnDelay = 5.f;

	// PlayerStart의 Player Start Tag(NewField는 현재 None)
	UPROPERTY(EditAnywhere, Category = "Respawn")
	FName RespawnStartTag = FName(TEXT("None"));

	// 부활 시 카메라 방향을 Player Start와 일치 시킬지
	UPROPERTY(EditAnywhere, Category = "Respawn")
	bool bResetControlRotation = true;

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCPHealthComponent> HealthComponent;
	
	UPROPERTY(Transient)
	TObjectPtr<UCPInteractionComponent> interactionComponent;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> RespawnAnimationMesh;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerStart> CachedRespawnStart;

	FTimerHandle RespawnTimerHandle;
};
