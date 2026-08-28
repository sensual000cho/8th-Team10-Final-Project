// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPCauldronComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"
#include "GameMode/CPLabGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Lab/Actor/CPAlchemyProp.h"

UCPCauldronComponent::UCPCauldronComponent(): MaxSlotCount(3)
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionPrompt = FText::FromString(TEXT("포션 만들기"));
	bShowWhenUnavailable = true;
}

void UCPCauldronComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// BP 가마솥에 배치한 내부 Trigger 탐색
	BoundIngredientTrigger = Cast<UPrimitiveComponent>(IngredientTrigger.GetComponent(GetOwner()));

	if (BoundIngredientTrigger){
		BoundIngredientTrigger->OnComponentBeginOverlap.AddUniqueDynamic(
				this, &UCPCauldronComponent::HandleIngredientOverlap);
	}
}

void UCPCauldronComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{	
	if (BoundIngredientTrigger){
		BoundIngredientTrigger->OnComponentBeginOverlap.RemoveDynamic(
				this, &UCPCauldronComponent::HandleIngredientOverlap);

		BoundIngredientTrigger = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

bool UCPCauldronComponent::ExecuteInteraction(AActor* Interactor)
{
	if (!CanExecuteInteraction(Interactor)) return false;

	const UStaticMeshComponent* SpawnMesh = 
		Cast<UStaticMeshComponent>(PotionSpawnMesh.GetComponent(GetOwner()));
	if (!SpawnMesh) return false;
	
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(UGameplayStatics::GetGameMode(this));
	if (!LabGameMode || !LabGameMode->RefinePotion(GetEffectTags(), MakePotionTransform())) return false;
	
	IngredientInstances.Reset();
	return true;
}

bool UCPCauldronComponent::CanExecuteInteraction(AActor* Interactor) const
{
	if (!Super::CanExecuteInteraction(Interactor)) return false;

	return IngredientInstances.Num() > 0;
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

TArray<FCPLabIngredientInstance> UCPCauldronComponent::GetIngredientInstance() const
{
	return IngredientInstances;
}

bool UCPCauldronComponent::CanAcceptProp(const ACPAlchemyProp* Prop) const
{
	if (!IsValid(Prop) || MaxSlotCount <= 0 || IngredientInstances.Num() >= MaxSlotCount) return false;

	return Prop->GetWorkingIngredient().IsValid();
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

void UCPCauldronComponent::HandleIngredientOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACPAlchemyProp* IngredientProp = Cast<ACPAlchemyProp>(OtherActor);
	if (!CanAcceptProp(IngredientProp)) return;

	const FCPLabIngredientInstance Ingredient = IngredientProp->GetWorkingIngredient();
	
	// Slot에 넣고, Destroy
	IngredientInstances.Add(Ingredient);
	IngredientProp->SetActorEnableCollision(false);
	IngredientProp->Destroy();
	
	DebugPrintSlots();
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
