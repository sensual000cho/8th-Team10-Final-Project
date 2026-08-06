#pragma once

#include "CoreMinimal.h"
#include "CPNPCTypes.generated.h"

UENUM(BlueprintType)
enum class ECPNPCActivityState : uint8
{
	Idle,
	Talking,
	Crafting,
};

UENUM(BlueprintType)
enum class ECPNPCEmotion : uint8
{
	Neutral,
	Happy,
	Annoyed,
	Angry,
	Sad,
};

UENUM(BlueprintType)
enum class ECPNPCSituation : uint8
{
	Greeting,
	Farewell,
	QuestOffer,
};

USTRUCT(BlueprintType)
struct FCPNPCDialogueEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Lines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> Animation;
};

USTRUCT(BlueprintType)
struct FCPNPCSituationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ECPNPCEmotion, FCPNPCDialogueEntry> EmotionVariants;
};