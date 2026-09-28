#pragma once

#include "CoreMinimal.h"
#include "NPC/CPBaseNPC.h"
#include "CPShopNPC.generated.h"

class UCPNPCComponent;
class UCPShopContainerComponent;

UCLASS()
class CREATEPOTION_API ACPShopNPC : public ACPBaseNPC
{
	GENERATED_BODY()

public:
	ACPShopNPC();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void OpenShopUI(AActor* Interactor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCPNPCComponent> NPCComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCPShopContainerComponent> ShopContainerComponent;
};