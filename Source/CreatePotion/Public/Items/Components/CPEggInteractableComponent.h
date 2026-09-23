#pragma once

#include "CoreMinimal.h"
#include "Components/CPInteractableComponent.h"
#include "CPEggInteractableComponent.generated.h"

class UCPForageableItemData;
// 알 상호작용 컴포넌트
UCLASS()
class CREATEPOTION_API UCPEggInteractableComponent : public UCPInteractableComponent
{
	GENERATED_BODY()
	
public:
	void Initialize(UCPForageableItemData* InItemData, int32 InAmount);
	virtual bool TryInteract(AActor* Interactor);
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCPForageableItemData> ItemData;
	
	int32 Amount;
};
