// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPCauldronComponent.h"

#include "Data/CPForageableItemData.h"
#include "GameMode/CPLabGameMode.h"
#include "GameState/CPLabGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Lab/Component/CPLabPotionSessionComponent.h"

UCPCauldronComponent::UCPCauldronComponent(): MaxSlotCount(3)
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionPrompt = FText::FromString(TEXT("재료 넣기"));
}

void UCPCauldronComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (BoundPotionSession) return;
	
	const UWorld* World = GetWorld();
	const ACPLabGameState* LabState = World? World->GetGameState<ACPLabGameState>(): nullptr;
	BoundPotionSession = LabState? LabState->GetPotionSession(): nullptr;
	if (BoundPotionSession){
		BoundPotionSession->OnSessionChanged.AddUniqueDynamic(this, &UCPCauldronComponent::HandleSessionChanged);
	}
}

void UCPCauldronComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundPotionSession){
		BoundPotionSession->OnSessionChanged.RemoveDynamic(this, &UCPCauldronComponent::HandleSessionChanged);
		BoundPotionSession = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

bool UCPCauldronComponent::ExecuteInteraction(AActor* Interactor)
{
	if (!CanExecuteInteraction(Interactor) || !AddProp()) return false;
	
	if (IngredientInstances.Num() >= MaxSlotCount){
		return ConfirmPotion();
	}
	
	return true;
}

bool UCPCauldronComponent::CanExecuteInteraction(AActor* Interactor) const
{
	//return Super::CanExecuteInteraction(Interactor) && 
	//	BoundPotionSession && BoundPotionSession->HasActiveRequest() && IngredientInstances.Num() < MaxSlotCount;
	
	return Super::CanExecuteInteraction(Interactor) && 
		BoundPotionSession && IngredientInstances.Num() < MaxSlotCount;
}

TArray<FGameplayTag> UCPCauldronComponent::GetEffectTags() const
{
	TArray<FGameplayTag> CombinedTags;
	// 넣은 순서대로 효과를 추가
	for (const FCPLabIngredientInstance& Ingredient : IngredientInstances){
		if (!Ingredient.IsValid()) continue;
		
		for (const FGameplayTag& EffectTag : Ingredient.CurrentEffects){
			if (EffectTag.IsValid()){
				CombinedTags.Add(EffectTag);
			}
		}
	}
	
	return CombinedTags;
}

bool UCPCauldronComponent::AddProp()
{
	if (IngredientInstances.Num() >= MaxSlotCount) return false;
	
	ACPAlchemyProp* HeldProp = BoundPotionSession->GetHeldAlchemyProp();
	if (!IsValid(HeldProp)) return false;
	
	const FCPLabIngredientInstance Ingredient = HeldProp->GetWorkingIngredient();
	if (!Ingredient.IsValid()) return false;
	
	ACPAlchemyProp* ReleaseProp = nullptr;
	if (!BoundPotionSession->ReleaseHeldAlchemyProp(ReleaseProp) || ReleaseProp != HeldProp) return false;
	
	IngredientInstances.Add(Ingredient);
	HeldProp->Destroy();
	
	DebugPrintSlots();
	return true;
}

bool UCPCauldronComponent::ConfirmPotion()
{
	const UStaticMeshComponent* SpawnMesh =
		Cast<UStaticMeshComponent>(PotionSpawnMesh.GetComponent(GetOwner()));
	if (!SpawnMesh) return false;
	
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(UGameplayStatics::GetGameMode(this));
	if (!LabGameMode) return false;
	
	return LabGameMode->RefinePotion(GetEffectTags(), MakePotionTransform());
}

FTransform UCPCauldronComponent::MakePotionTransform() const
{
	const UStaticMeshComponent* SpawnMesh =
		Cast<UStaticMeshComponent>(PotionSpawnMesh.GetComponent(GetOwner()));
	if (!SpawnMesh) return FTransform::Identity;
	
	FTransform SpawnTransform = SpawnMesh->GetComponentTransform();
	
	FVector SpawnLocation = SpawnMesh->Bounds.Origin;
	SpawnLocation.Z += SpawnMesh->Bounds.BoxExtent.Z;
	
	SpawnTransform.SetLocation(SpawnLocation);
	return SpawnTransform;
}

void UCPCauldronComponent::HandleSessionChanged()
{
	// 리퀘스트 종료 시에만 초기화
	if (!BoundPotionSession->HasActiveRequest()){
		IngredientInstances.Reset();
	}
}

// UI 도입 or PR확인 이후 삭제
void UCPCauldronComponent::DebugPrintSlots() const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Yellow,
			FString::Printf(TEXT("[가마솥] 재료 개수: %d / %d"),
				IngredientInstances.Num(),
				MaxSlotCount));
	}

	for (int32 Index = 0; Index < IngredientInstances.Num(); ++Index)
	{
		const FCPLabIngredientInstance& Ingredient = IngredientInstances[Index];

		const FString ItemName = Ingredient.SourceItemData
			? Ingredient.SourceItemData->DisplayName.ToString()
			: TEXT("없음");

		FString EffectText;
		for (const FGameplayTag& EffectTag : Ingredient.CurrentEffects)
		{
			if (!EffectTag.IsValid()) continue;

			if (!EffectText.IsEmpty())
			{
				EffectText += TEXT(", ");
			}

			EffectText += EffectTag.ToString();
		}

		const FString Message = FString::Printf(
			TEXT("[가마솥] 슬롯 %d: %s / 효과: %s"),
			Index,
			*ItemName,
			EffectText.IsEmpty() ? TEXT("없음") : *EffectText);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.f,
				FColor::Cyan,
				Message);
		}
	}
}
