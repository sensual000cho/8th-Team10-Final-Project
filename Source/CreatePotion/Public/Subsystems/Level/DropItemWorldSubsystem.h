// DropItemWorldSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DropItemWorldSubsystem.generated.h"

UCLASS()
class CREATEPOTION_API UDropItemWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	// 현재 Level에 드롭되어 있는 아이템을 관리할 WorldSubsystem
};
