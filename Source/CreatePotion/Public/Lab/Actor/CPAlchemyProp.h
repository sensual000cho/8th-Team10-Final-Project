 #pragma once

#include "CoreMinimal.h"
#include "Lab/Actor/CPThrowablePropBase.h"
#include "CPAlchemyProp.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class CREATEPOTION_API ACPAlchemyProp
	: public ACPThrowablePropBase
{
	GENERATED_BODY()

public:
	ACPAlchemyProp();

	virtual void Tick(float DeltaSeconds) override;
	
	virtual FName GetInteractionName_Implementation() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation")
	TObjectPtr<USceneComponent> IngredientUprightPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation")
	TObjectPtr<USceneComponent> IngredientBobblePivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation")
	TObjectPtr<UStaticMeshComponent> IngredientMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation")
	bool bEnableIngredientBobble = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation", meta = (ClampMin = "0.0", Units = "cm"))
	float BobbleAmplitude = 2.5f;

	// 초당 부유 왕복 횟수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation", meta = (ClampMin = "0.0", Units = "Hz"))
	float BobbleSpeed = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lab|Ingredient|Presentation", meta = (ClampMin = "0.0"))
	float UprightRecoverySpeed = 0.9f;

private:
	FVector IngredientBobbleBaseLocation = FVector::ZeroVector;
};
