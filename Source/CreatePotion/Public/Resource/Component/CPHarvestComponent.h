#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "Resource/CPResourceType.h"
#include "CPHarvestComponent.generated.h"


class UCPResourceDefinition;
struct FCPResourceNodeKey;
class UCPForageableItemData;

// 채집물 / 농작물용 수확 컴포넌트
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CREATEPOTION_API UCPHarvestComponent : public UCPInteractableComponent
{
	GENERATED_BODY()

public:	
	// Life Cycle
	UCPHarvestComponent();

	void Initialize(const FCPResourceNodeKey& InNodeKey, UCPResourceDefinition* InDefinition, int32 InHarvestMultiplier);
	
	virtual void BeginFocus(AActor* Interactor) override;
	virtual bool CanInteract() const override;
	virtual void BeginTimeInteraction(AActor* Interactor) override;
	virtual bool TryInteract(AActor* Interactor) override;
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UCPResourceDefinition> ResourceDefinition;
	
	FCPResourceNodeKey NodeKey;
	int32 HarvestMultiplier = 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Resource|Animation")
	TObjectPtr<UAnimMontage> HarvestMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Resource|Sound")
	TObjectPtr<USoundBase> HarvestSound;
};
