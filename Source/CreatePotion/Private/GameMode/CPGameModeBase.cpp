// CPGameModeBase.cpp

#include "GameMode/CPGameModeBase.h"
#include "Character/CPCharacter.h"
#include "Character/CPPlayerController.h"

ACPGameModeBase::ACPGameModeBase()
{
	DefaultPawnClass = ACPCharacter::StaticClass();
	PlayerControllerClass = ACPPlayerController::StaticClass();
}
