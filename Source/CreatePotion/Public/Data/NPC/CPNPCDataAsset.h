#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CPNPCDataAsset.generated.h"

class USkeletalMesh;
class UAnimSequence;
class UTexture2D;
class UStaticMesh;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct FCPSlimeAccessoryData
{
	GENERATED_BODY()

	// 부착할 액세서리 메시
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slime|Accessory")
	TSoftObjectPtr<UStaticMesh> AccessoryMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slime|Accessory")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slime|Accessory")
	FTransform RelativeTransform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct FCPHeatColorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat|Color")
	FLinearColor Color1 = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat|Color")
	FLinearColor Color2 = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat|Color")
	FLinearColor Color3 = FLinearColor::White;
};
UCLASS(BlueprintType)
class CREATEPOTION_API UCPNPCDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FName NPCName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Codex")
	TSoftObjectPtr<UTexture2D> CodexImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Codex")
	FName CodexTextKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Mesh")
	TSoftObjectPtr<USkeletalMesh> NPCMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Capsule")
	float CapsuleRadiusRatio = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh")
	FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Mesh")
	FRotator MeshRotationOffset;

	//날아가는 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	TSoftObjectPtr<UAnimSequence> LaunchAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Animation")
	TSubclassOf<UAnimInstance> AnimClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Bouncy")
	FLinearColor SlimeInnerColor = FLinearColor(0.02f, 0.25f, 0.05f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Potion|Bouncy")
	FLinearColor SlimeOuterColor = FLinearColor(0.2f, 1.0f, 0.4f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Slime")
	FCPSlimeAccessoryData SlimeAccessory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Heat")
	FCPHeatColorData HeatColorData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Potion|Visual", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float VisualScaleMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Move")
	FDataTableRowHandle TargetRowHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Move")
	float MoveSpeed = 200.0f;

};