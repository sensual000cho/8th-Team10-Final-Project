// CPEnemyAIController.h

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CPEnemyAIController.generated.h"

class UStateTree;
class UStateTreeAIComponent;

UCLASS()
class CREATEPOTION_API ACPEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ACPEnemyAIController();
	
	UFUNCTION()
	FVector GetHomeLocation() const { return HomeLocation; }
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;
	
	// 자식 AIController BP의 Class Defaults에서 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|AI")
	TObjectPtr<UStateTree> EnemyStateTree;
	
private:
	void TryStartStateTree();
	
	UPROPERTY(VisibleInstanceOnly, Category = "Enemy|AI")
	FVector HomeLocation = FVector::ZeroVector;
};
