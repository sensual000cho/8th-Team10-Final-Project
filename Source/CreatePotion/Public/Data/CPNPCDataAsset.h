#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NPC/CPNPCTypes.h"
#include "CPNPCDataAsset.generated.h"

class USkeletalMesh;
class UAnimSequence;

UCLASS()
class CREATEPOTION_API UCPNPCDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "NPC")
	FName NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	TSoftObjectPtr<USkeletalMesh> NPCMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Capsule")
	float CapsuleRadiusRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh")
	FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh")
	FRotator MeshRotationOffset;

	// 현재 바로 사용할 기본 Idle 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	TSoftObjectPtr<UAnimSequence> IdleAnimation;

	//임시 2차 프로토용 (날아가는 애니메이션)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	TSoftObjectPtr<UAnimSequence> LaunchAnimation;

	// [마을 전용] 부여할 마을 퀘스트 ID 목록 (DT_QuestScript의 RowName 참조)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Quest")
	TArray<FName> TownQuestIDs;

	// [공방 전용] 담당할 공방 퀘스트 ID 목록 (DT_QuestAnswer의 RowName 참조)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Quest")
	TArray<FName> LabQuestIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Sound")
	USoundBase* FartSound;
};