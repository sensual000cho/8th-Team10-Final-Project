#include "Lab/Actor/CPAlchemyProp.h"

#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"
#include "GameState/CPLabGameState.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

// 물리 재료 Actor의 기본 컴포넌트 구성
ACPAlchemyProp::ACPAlchemyProp()
{
	// 직접 이동·가공 요청을 받을 때만 갱신하므로 Tick은 사용하지 않음
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(StaticMeshComponent);
}

void ACPAlchemyProp::OnInteract_Implementation(AActor* Interactor)
{
	UWorld* World = GetWorld();
	ACPLabGameState* LabGameState = World? World->GetGameState<ACPLabGameState>(): nullptr;
	UCPLabPotionSessionComponent* Session = LabGameState? LabGameState->GetPotionSession(): nullptr;
	if (!Session) return;
	
	// 손에 들고 있던 Prop이 있던 경우
	ACPAlchemyProp* PreviousHeldProp = nullptr;
	if (Session->ReleaseHeldAlchemyProp(PreviousHeldProp) && IsValid(PreviousHeldProp)){
		FVector DropLocation = Interactor ? Interactor->GetActorForwardVector() * 100.0f : FVector::ZeroVector;
		DropLocation += Interactor->GetActorLocation();
		
		PreviousHeldProp->SetActorLocation(DropLocation);
		PreviousHeldProp->SetActorHiddenInGame(false);
		PreviousHeldProp->SetActorEnableCollision(true);
	}
	
	// 상호작용한 Prop을 손에 들고 비활성화
	if (!Session->HoldAlchemyProp(this)) return;
	
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

FText ACPAlchemyProp::GetInteractionPrompt_Implementation()
{
	return FText::FromString(TEXT("재료 들기"));
}

FName ACPAlchemyProp::GetInteractionName_Implementation()
{
	return FName(TEXT("Prop"));
}

bool ACPAlchemyProp::CanInteract_Implementation(AActor* Interactor)
{
	return true;
}

void ACPAlchemyProp::InitializeFromItemData(UCPForageableItemData* ItemData)
{
	InitializeAlchemyProp(ItemData);
}

void ACPAlchemyProp::InitializeAlchemyProp(UCPForageableItemData* ItemData, const TArray<FGameplayTag>& EffectTags)
{
	WorkingIngredient = FCPLabIngredientInstance{};
	
	if (!ItemData) return;
	
	// 배열이 들어오면 배열, 들어오지 않으면 DA을 사용해 초기화
	WorkingIngredient.SourceItemData = ItemData;
	if (EffectTags.Num() > 0){
		WorkingIngredient.CurrentEffects = EffectTags;	
	}else{
		WorkingIngredient.CurrentEffects = ItemData->TagAxes;
	}
}

FCPLabIngredientInstance
ACPAlchemyProp::GetWorkingIngredient() const
{
	return WorkingIngredient;
}