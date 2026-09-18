#include "NPC/CPLabNPC.h"
#include "GameMode/CPLabGameMode.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"
#include "GameInstance/Subsystem/CPNPCSubsystem.h"

void ACPLabNPC::ShowResultDialogue()
{
	Super::ShowResultDialogue();

	if (ActiveDialogueWidget) {
		ActiveDialogueWidget->OnClosed.AddUObject(this, &ACPLabNPC::OnResultDialogueClosed);
	}
}

void ACPLabNPC::BeginPlay()
{
	Super::BeginPlay();
	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem()){
		Subsystem->SetNPCActiveStateById(GetPotionNPCId(), false, this);
	}
}

void ACPLabNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem()){
		Subsystem->SetNPCActiveStateById(GetPotionNPCId(), true, this);
	}

	Super::EndPlay(EndPlayReason);
}

void ACPLabNPC::OnResultDialogueClosed()
{
	if (UWorld* World = GetWorld()) {
		if (ACPLabGameMode* LabGameMode = Cast<ACPLabGameMode>(World->GetAuthGameMode())) {
			LabGameMode->ClearPotionRequest();
		}
	}
	if (UCPNPCSubsystem* Subsystem = GetNPCSubsystem())
	{
		Subsystem->SetNPCActiveStateById(GetPotionNPCId(), true, this);
	}
	Destroy();
}