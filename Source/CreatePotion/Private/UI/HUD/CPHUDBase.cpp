// CPHUDBase.cpp

#include "UI/HUD/CPHUDBase.h"
#include "Blueprint/UserWidget.h"
#include "GameInstance/Subsystem/CPCodexSubsystem.h"
#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "Settings/CPUISettings.h"
#include "UI/Widgets/Common/Notification/CPCommonNotificationWidget.h"
#include "Data/CPForageableItemData.h"

void ACPHUDBase::BeginPlay()
{
	Super::BeginPlay();
	
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[CPHUDBase]GameInstance가 유효하지 않습니다."));
		return;
	}
	CachedUIManager = GI->GetSubsystem<UCPUIManagerSubsystem>();
	if (!CachedUIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[CPHUDBase]CachedUIManager가 유효하지 않습니다."));
		return;
	}
	
	if (MainHUDWidgetClass)
	{
		CachedUIManager->PushWidget(MainHUDWidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] MainHUDWidgetClass가 지정되지 않았습니다."), *GetName());
	}

	
	const UCPUISettings* UISettings = GetDefault<UCPUISettings>();
	if (UISettings)
	{
		NotificationWidgetClass = UISettings->NotificationWidgetClass.LoadSynchronous();	
	}
	
	if(!IsValid(NotificationWidgetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CPHUDBase]NotificationWidgetClass가 유효하지 않습니다."));
	}
	
	
	OnMainHUDWidgetCreated();
	
	
}

void ACPHUDBase::HandleForageableCodexRegistered(UCPForageableItemData* ItemData)
{
	UE_LOG(LogTemp, Warning, TEXT("[CPHUDBase]ForageableCodexUpdated"));
	
	UCPCommonNotificationWidget* NotificationWidget = Cast<UCPCommonNotificationWidget>(CreateWidget(GetOwningPlayerController(), NotificationWidgetClass));
	if (!IsValid(NotificationWidget)) return;
	
	FFormatNamedArguments Args;
	Args.Add(TEXT("Item"), ItemData->DisplayName);
	
	const FText Message = FText::Format(
		NSLOCTEXT("Notice", "ForageableRegistered", "{Item}|hpp(을,를) 도감에 기록했어요"), 
		Args);
	NotificationWidget->SetNotificationText(Message);
	NotificationWidget->ShowNotification();
}

void ACPHUDBase::HandleNPCCodexRegistered(UCPNPCDataAsset* NPCData)
{
}

void ACPHUDBase::OnMainHUDWidgetCreated()
{
	UGameInstance* GI = GetGameInstance();
	UCPCodexSubsystem* CodexSubsystem = GI->GetSubsystem<UCPCodexSubsystem>();
	if (!CodexSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[CPHUDBase]CodexSubsystem이 유효하지 않습니다."));
		return;
	}
	
	CodexSubsystem->OnForageableCodexRegistered.AddDynamic(this, &ACPHUDBase::HandleForageableCodexRegistered);
	CodexSubsystem->OnNPCCodexRegistered.AddDynamic(this, &ACPHUDBase::HandleNPCCodexRegistered);
}
