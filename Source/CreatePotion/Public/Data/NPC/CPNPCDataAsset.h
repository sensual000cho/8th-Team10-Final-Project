#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CPNPCDataAsset.generated.h"

class USkeletalMesh;
class UAnimSequence;

UCLASS(BlueprintType)
class CREATEPOTION_API UCPNPCDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FName NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Mesh")
	TSoftObjectPtr<USkeletalMesh> NPCMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Capsule")
	float CapsuleRadiusRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh")
	FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh")
	FRotator MeshRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	TSoftObjectPtr<UAnimSequence> IdleAnimation;

	//날아가는 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	TSoftObjectPtr<UAnimSequence> LaunchAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Sound")
	USoundBase* FartSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Bouncy")
	FLinearColor SlimeInnerColor = FLinearColor(0.02f, 0.25f, 0.05f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Bouncy")
	FLinearColor SlimeOuterColor = FLinearColor(0.2f, 1.0f, 0.4f, 1.0f);
};