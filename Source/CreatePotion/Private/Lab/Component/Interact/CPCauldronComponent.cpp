// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPCauldronComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/CPForageableItemData.h"
#include "Data/CPTagDefinitionTypes.h"
#include "Engine/DataTable.h"
#include "GameMode/CPLabGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Lab/Actor/CPAlchemyProp.h"
#include "Settings/CPDTSettings.h"

UCPCauldronComponent::UCPCauldronComponent(): 
	MaxSlotCount(3), 
	UpImpulse(5000.f), 
	RandomImpulseMin(2000.f),
	RandomImpulseMax(4000.f)
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
	if (!BoundIngredientTrigger) return false;
	
	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(UGameplayStatics::GetGameMode(this));
	if (!LabGameMode || !LabGameMode->CreatePotion(GetEffectTags(), MakePotionTransform(), MakeSpawnImpulse())) return false;
	
	IngredientInstances.Reset();
	IngredientTags.Reset();
	return true;
}

bool UCPCauldronComponent::CanExecuteInteraction(AActor* Interactor) const
{
	if (!Super::CanExecuteInteraction(Interactor)) return false;

	return IngredientInstances.Num() > 0;
}

TArray<FGameplayTag> UCPCauldronComponent::GetEffectTags() const
{
	return IngredientTags;
}

TArray<FCPLabIngredientInstance> UCPCauldronComponent::GetIngredientInstance() const
{
	return IngredientInstances;
}

bool UCPCauldronComponent::CanAcceptProp(const ACPAlchemyProp* Prop) const
{
	if (!IsValid(Prop) || MaxSlotCount <= 0 || IngredientInstances.Num() >= MaxSlotCount) return false;

	const FCPLabIngredientInstance& Ingredient = Prop->GetWorkingIngredient();
	return Ingredient.IsValid() && Ingredient.CurrentEffects.IsEmpty();
}

void UCPCauldronComponent::ResolveTagCombinations(int32 NewTagIndex)
{
	// 새로운 태그 or 조합 태그로 첫번째 Tag 까지 탐색
	while (IngredientTags.IsValidIndex(NewTagIndex)){
		const FGameplayTag NewTag = IngredientTags[NewTagIndex];
		const FCPTagDefinitionRow* NewTagRow = nullptr;
		if (!FindTagDefinitionRow(NewTag, NewTagRow) || !NewTagRow) break;
		
		bool bCombined = false;
		
		// 뒤부터 첫번째 Tag 까지 순회
		for (int32 CandidateIndex = NewTagIndex - 1; CandidateIndex >= 0; --CandidateIndex){
			const FGameplayTag CandidateTag = IngredientTags[CandidateIndex];
			
			// 새로운 태그와 CandidateIndex의 태그가 조합 가능한 조합이 있는지 확인
			for (const FCPTagCombinationEntry& Combination : NewTagRow->Combinations){
				if (Combination.OtherTag != CandidateTag || !Combination.ResultTag.IsValid()) continue;
				
				const FGameplayTag ResultTag = Combination.ResultTag;
				
				// 조합 태그 제거
				IngredientTags.RemoveAt(NewTagIndex);
				IngredientTags.RemoveAt(CandidateIndex);
			
				// 동일한 태그가 있을 경우 제외
				if (!IngredientTags.Contains(ResultTag)){
					IngredientTags.Add(ResultTag);
					NewTagIndex = IngredientTags.Num() - 1;
					bCombined = true;
				}
				break;
			}
			if (bCombined) break;
		}
		if (!bCombined) break;
	}
}

bool UCPCauldronComponent::FindTagDefinitionRow(const FGameplayTag& Tag, const FCPTagDefinitionRow*& OutRow) const
{
	OutRow = nullptr;
	if (!Tag.IsValid()) return false;
	
	const UCPDTSettings* DTSettings = GetDefault<UCPDTSettings>();
	if (!DTSettings) return false;
	
	UDataTable* TagDefinitionTable = DTSettings->TagDefinitionTable.LoadSynchronous();
	if (!TagDefinitionTable) return false;
	
	TArray<FCPTagDefinitionRow*> Rows;
	TagDefinitionTable->GetAllRows<FCPTagDefinitionRow>(TEXT("CauldronTagDefinition"), Rows);
	
	for (const FCPTagDefinitionRow* Row : Rows){
		if (Row && Row->Tag == Tag){
			OutRow = Row;
			return true;
		}
	}
	
	return false;
}

FTransform UCPCauldronComponent::MakePotionTransform() const
{
	if (!BoundIngredientTrigger) return FTransform::Identity;
	
	FTransform SpawnTransform = BoundIngredientTrigger->GetComponentTransform();
	FVector SpawnLocation = BoundIngredientTrigger->Bounds.Origin;
	SpawnLocation.Z += BoundIngredientTrigger->Bounds.BoxExtent.Z;
	
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetScale3D(FVector::OneVector);
	return SpawnTransform;
}

FVector UCPCauldronComponent::MakeSpawnImpulse() const
{
	const float RandomAngle = FMath::RandRange(0.f, 2.f * PI);
	const float RandomStrength = FMath::RandRange(RandomImpulseMin, RandomImpulseMax);
	
	const FVector RandomHorizontalImpulse(
		FMath::Cos(RandomAngle) * RandomStrength, 
		FMath::Sin(RandomAngle) * RandomStrength, 
		0.f
	);
	
	return FVector::UpVector * UpImpulse + RandomHorizontalImpulse;
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
	if (Ingredient.SourceItemData && !Ingredient.SourceItemData->TagAxes.IsEmpty()){
		const FGameplayTag IngredientTag = Ingredient.SourceItemData->TagAxes[0];
		
		if (IngredientTag.IsValid() && !IngredientTags.Contains(IngredientTag)){
			IngredientTags.Add(IngredientTag);
			ResolveTagCombinations(IngredientTags.Num() - 1);
		}
	}
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
		if (Ingredient.SourceItemData){
			for (const FGameplayTag& EffectTag : Ingredient.SourceItemData->TagAxes)
			{
				if (!EffectTag.IsValid()) continue;

				if (!EffectText.IsEmpty())
				{
					EffectText += TEXT(", ");
				}

				EffectText += EffectTag.ToString();
			}	
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
