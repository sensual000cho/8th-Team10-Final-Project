// CPGameModeBase.cpp

#include "GameMode/CPGameModeBase.h"
#include "Character/CPCharacter.h"
#include "Character/CPPlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameInstance/Subsystem/CPLevelTravelSubsystem.h"
#include "GameInstance/Subsystem/CPTimeSubsystem.h"

ACPGameModeBase::ACPGameModeBase()
{
	DefaultPawnClass = ACPCharacter::StaticClass();
	PlayerControllerClass = ACPPlayerController::StaticClass();
}

AActor* ACPGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UCPLevelTravelSubsystem* TravelSubsystem = 
		GetGameInstance()? GetGameInstance()->GetSubsystem<UCPLevelTravelSubsystem>(): nullptr;

	const FName RequestedSpawnId =
		TravelSubsystem	? TravelSubsystem->GetPendingSpawnPointId()	: NAME_None;

	// 게이트에서 스폰 지점을 지정한 경우
	if (!RequestedSpawnId.IsNone())
	{
		APlayerStart* RequestedPlayerStart =
			FindPlayerStartByTag(RequestedSpawnId);

		if (RequestedPlayerStart)
		{
			TravelSubsystem->ConsumePendingSpawnPointId();
			return RequestedPlayerStart;
		}

		UE_LOG(LogTemp,	Warning, TEXT("Requested PlayerStart not found: %s"), *RequestedSpawnId.ToString());

		// 잘못된 요청이 다음 레벨까지 남지 않도록 제거
		TravelSubsystem->ConsumePendingSpawnPointId();
	}

	// 레벨에서 지정한 기본 스폰 지점
	const FName DefaultSpawnPointId(TEXT("Default"));

	APlayerStart* DefaultPlayerStart = FindPlayerStartByTag(DefaultSpawnPointId);

	if (DefaultPlayerStart)
	{
		return DefaultPlayerStart;
	}
	
	// Default 태그도 없다면 Unreal 기본 로직 사용
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ACPGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	UCPTimeSubsystem* TimeSubsystem = GetGameInstance()->GetSubsystem<UCPTimeSubsystem>();
	if (TimeSubsystem)
	{
		TimeSubsystem->StartTime();
	}
}

APlayerStart* ACPGameModeBase::FindPlayerStartByTag(FName SpawnPointId) const
{
	if (SpawnPointId.IsNone() || !GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<APlayerStart> PlayerStartIt(GetWorld()); PlayerStartIt; ++PlayerStartIt)
	{
		APlayerStart* PlayerStart = *PlayerStartIt;

		if (IsValid(PlayerStart) &&	PlayerStart->PlayerStartTag == SpawnPointId)
		{
			return PlayerStart;
		}
	}

	return nullptr;
}
