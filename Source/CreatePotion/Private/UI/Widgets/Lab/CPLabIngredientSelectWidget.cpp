// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/Lab/CPLabIngredientSelectWidget.h"

#include "Character/CPPlayerController.h"
#include "Components/CPInventoryComponent.h"
#include "Components/CPItemContainerComponent.h"
#include "Components/CPLabContainerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widgets/Common/Container/CPContainerMainWidget.h"
#include "GameFramework/Character.h"
#include "Lab/Actor/CPLabContainerActor.h"
#include "Components/Button.h"


void UCPLabIngredientSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	UCPItemContainerComponent* Inventory = PlayerCharacter ? PlayerCharacter->FindComponentByClass<UCPItemContainerComponent>() : nullptr;
	
	AActor* LabActor = UGameplayStatics::GetActorOfClass(GetWorld(), ACPLabContainerActor::StaticClass());
	UCPLabContainerComponent* SelectSlot = LabActor ? LabActor->FindComponentByClass<UCPLabContainerComponent>() : nullptr;

	// if (InventoryWidget && Inventory)
	// {
	// 	InventoryWidget->BindContainer(Inventory);
	// 	UE_LOG(LogTemp, Warning, TEXT("[재료 선택 위젯] 인벤토리 위젯 바인딩 성공"));
	// }
	
	// 디버그용 아이템 추가
	AddTestItems();
}

void UCPLabIngredientSelectWidget::BindEvents()
{
	Super::BindEvents();
	if (Button_Confirm)
	{
		Button_Confirm->OnClicked.AddDynamic(this, &UCPLabIngredientSelectWidget::OnConfirmClicked);
	}
}

void UCPLabIngredientSelectWidget::OnConfirmClicked()
{
	OnIngredientConfirmed.Broadcast();
	RequestClose();
	
	if (ACPPlayerController* PC = GetOwningPlayer<ACPPlayerController>())
	{
		PC->ToggleLabUI();
	}
}

void UCPLabIngredientSelectWidget::AddTestItems()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter) return;
	
	// 캐릭터에서 인벤토리 컴포넌트 찾아내기
	UCPInventoryComponent* InventoryComp = PlayerCharacter->FindComponentByClass<UCPInventoryComponent>();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Error, TEXT("[IngredientSelectWidget] 캐릭터 인벤토리 컴포넌트 미발견"));
		return;
	}
	
	if (TestItemDatas.Num() > 0)
	{
		for (UCPForageableItemData* ItemData : TestItemDatas)
		{
			if (ItemData)
			{
				InventoryComp->TryGetItem(ItemData, 1);
			}
		}
	}
}


