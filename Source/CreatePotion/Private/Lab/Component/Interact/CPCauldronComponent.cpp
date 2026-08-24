// Fill out your copyright notice in the Description page of Project Settings.


#include "Lab/Component/Interact/CPCauldronComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
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
	
	// BP 가마솥에 배치한 내부 Trigger 탐색
	BoundIngredientTrigger = Cast<UPrimitiveComponent>(IngredientTrigger.GetComponent(GetOwner()));

	if (BoundIngredientTrigger)
	{
		BoundIngredientTrigger->OnComponentBeginOverlap.AddUniqueDynamic(
				this,
				&UCPCauldronComponent::HandleIngredientOverlap);
	}
}

void UCPCauldronComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundPotionSession){
		BoundPotionSession->OnSessionChanged.RemoveDynamic(this, &UCPCauldronComponent::HandleSessionChanged);
		BoundPotionSession = nullptr;
	}
	
	if (BoundIngredientTrigger)
	{
		BoundIngredientTrigger->OnComponentBeginOverlap.RemoveDynamic(
				this,
				&UCPCauldronComponent::HandleIngredientOverlap);

		BoundIngredientTrigger = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

bool UCPCauldronComponent::ExecuteInteraction(AActor* Interactor)
{
	if (!CanExecuteInteraction(Interactor))
	{
		return false;
	}

	if (!AddHeldProp())
	{
		return false;
	}

	return ConfirmPotionIfReady();
}

bool UCPCauldronComponent::CanExecuteInteraction(AActor* Interactor) const
{
	if (!Super::CanExecuteInteraction(Interactor) || !BoundPotionSession)
	{
		return false;
	}

	ACPAlchemyProp* HeldProp = BoundPotionSession->GetHeldAlchemyProp();

	return CanAcceptProp(HeldProp);
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

bool UCPCauldronComponent::AddHeldProp()
{
	if (!BoundPotionSession)
	{
		return false;
	}

	ACPAlchemyProp* HeldProp = BoundPotionSession->GetHeldAlchemyProp();

	if (!CanAcceptProp(HeldProp))
	{
		return false;
	}

	ACPAlchemyProp* ReleasedProp = nullptr;

	if (!BoundPotionSession->ReleaseHeldAlchemyProp(ReleasedProp))
	{
		return false;
	}

	// 일반적으로 발생하지 않지만 상태 불일치 방어
	if (ReleasedProp != HeldProp)
	{
		if (IsValid(ReleasedProp))
		{
			BoundPotionSession->HoldAlchemyProp(ReleasedProp);
		}

		return false;
	}

	if (!AddProp(ReleasedProp))
	{
		// 아직 파괴되지 않았으므로 Session 상태 복구
		BoundPotionSession->HoldAlchemyProp(ReleasedProp);
		return false;
	}

	return true;
}

bool UCPCauldronComponent::AddProp(ACPAlchemyProp* Prop)
{
	if (!CanAcceptProp(Prop))
	{
		return false;
	}

	const FCPLabIngredientInstance Ingredient = Prop->GetWorkingIngredient();

	// 먼저 슬롯에 데이터 복사
	IngredientInstances.Add(Ingredient);

	// 중복 Overlap 또는 추가 상호작용 방지
	Prop->SetActorEnableCollision(false);

	// 머리 위에 부착되어 있거나 물리 시뮬레이션 중이어도
	// Destroy하면 자동으로 제거
	Prop->Destroy();

	DebugPrintSlots();
	return true;
}

bool UCPCauldronComponent::CanAcceptProp(
	const ACPAlchemyProp* Prop) const
{
	if (!BoundPotionSession || !IsValid(Prop) || MaxSlotCount <= 0 || IngredientInstances.Num() >= MaxSlotCount)
	{
		return false;
	}
	
	if (!BoundPotionSession->HasActiveRequest())
	{
		return false;
	}

	return Prop->GetWorkingIngredient().IsValid();
}

bool UCPCauldronComponent::ConfirmPotionIfReady()
{
	if (IngredientInstances.Num() < MaxSlotCount)
	{
		return true;
	}

	return ConfirmPotion();
}

bool UCPCauldronComponent::ConfirmPotion()
{
	const UStaticMeshComponent* SpawnMesh = Cast<UStaticMeshComponent>(PotionSpawnMesh.GetComponent(GetOwner()));

	if (!SpawnMesh)
	{
		return false;
	}

	ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(UGameplayStatics::GetGameMode(this));

	if (!LabGameMode)
	{
		return false;
	}

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

void UCPCauldronComponent::HandleIngredientOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACPAlchemyProp* Ingredient =
		Cast<ACPAlchemyProp>(OtherActor);

	if (!CanAcceptProp(Ingredient))
	{
		return;
	}

	/*
	 * 정상적인 투척 흐름이라면 투척 순간 Session에서 이미
	 * HeldAlchemyProp이 해제되어 있어야 한다.
	 *
	 * 아직 Session이 들고 있다고 판단되는 액터는
	 * 머리 위에 있는 Held된 재료일 수 있으므로 받지 않음.
	 */
	if (BoundPotionSession->GetHeldAlchemyProp() == Ingredient)
	{
		return;
	}

	if (!AddProp(Ingredient))
	{
		return;
	}

	ConfirmPotionIfReady();
}

void UCPCauldronComponent::HandleSessionChanged()
{
	if (!BoundPotionSession)
	{
		return;
	}
	// 리퀘스트 종료 시에만 초기화
	if (!BoundPotionSession->HasActiveRequest())
	{
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
