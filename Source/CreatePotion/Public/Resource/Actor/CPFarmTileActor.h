#pragma once

#include "CoreMinimal.h"
#include "GameCore/Interface/CPInteractable.h"
#include "GameCore/Interface/CPTimedInteractable.h"
#include "GameFramework/Actor.h"
#include "Resource/CPFarmType.h"
#include "CPFarmTileActor.generated.h"

// 개별 농지 액터
UCLASS()
class CREATEPOTION_API ACPFarmTileActor : public AActor, public ICPInteractable, public ICPTimedInteractable
{
	GENERATED_BODY()
	
public:	
	ACPFarmTileActor();
	
	// 인터랙션 인터페이스 관련
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	
	virtual FText GetInteractionPrompt_Implementation() override;
	
	virtual FName GetInteractionName_Implementation() override;
	
	virtual bool CanInteract_Implementation(AActor* Interactor) override;
	
	virtual float GetInteractionDuration_Implementation(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SoilMesh;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CropMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Farm")
	FName FarmId = TEXT("Farm");
	
	UPROPERTY(EditDefaultsOnly, Category = "Farm")
	FIntPoint GridCoordinate;
	
private:
	FCPFarmTileKey GetTileKey() const;
	
	// 외형 최신화
	void RefreshVisual();
};
