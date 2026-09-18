#include "Quest/Components/CPStoryActorComponent.h"

#include "GameInstance/Subsystem/CPUIManagerSubsystem.h"
#include "UI/Widgets/Common/Dialogue/CPNPCDialogueWidget.h"

UCPStoryActorComponent::UCPStoryActorComponent()
{
	InteractionPrompt = FText::FromString(TEXT("F : 읽기"));
	TargetName = FName(TEXT("StoryText"));
}

bool UCPStoryActorComponent::TryInteract(AActor* Interactor)
{
	if (!Super::TryInteract(Interactor)) return false;
	if (!DialogueWidgetClass || LoreLines.IsEmpty()) return false;

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GameInstance) return false;

	UCPUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UCPUIManagerSubsystem>();
	if (!UIManager) return false;

	UCPNPCDialogueWidget* Widget = Cast<UCPNPCDialogueWidget>(UIManager->PushWidget(DialogueWidgetClass));
	if (!Widget) return false;

	Widget->InitDialogueLines(NAME_None, DisplayName, LoreLines);
	return true;
}
