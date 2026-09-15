// DropItemWorldSubsystem.h
// 현재 Level에 드롭되어 있는 아이템을 관리할 WorldSubsystem

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DropItemWorldSubsystem.generated.h"

class ACPAlchemyProp;
class UCPItemContainerComponent;

UCLASS()
class CREATEPOTION_API UDropItemWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void RegisterDroppedProp(ACPAlchemyProp* Prop);

	UFUNCTION()
	void UnregisterDroppedProp(ACPAlchemyProp* Prop);

	UFUNCTION(BlueprintCallable, Category = "Drop")
	void CollectAllDroppedItems(UCPItemContainerComponent* TargetContainer);

private:
	UPROPERTY()
	TArray<ACPAlchemyProp*> ActiveDroppedProps;
};
